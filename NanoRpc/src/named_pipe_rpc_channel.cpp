// This implementation uses exclusively asynchronous I/O for the reason of
// properly handling surprise disconnect condition.

#include "named_pipe_rpc_channel.h"

#include <cstdint>
#include <iostream>

#include <windows.h>

#include "named_pipe_connector.h"

namespace nanorpc2 {

NamedPipeRpcChannel *NamedPipeRpcChannelBuilder::Build() {
  return new NamedPipeRpcChannel(*this);
}

NamedPipeRpcChannel::~NamedPipeRpcChannel() {
  try {
    Close();
  }
  catch (...) {
  }
}

bool NamedPipeRpcChannel::Connect() {
  NamedPipeConnector connector;
  connector.set_pipe_name(builder_.get_pipe_name().c_str());
  connector.set_server_name(builder_.get_computer_name().c_str());
  connector.set_asynchronous(false);
  if (connector.StartConnection(builder_.get_client_side()))
    return ConnectInternal(connector.get_pipe());

  return false;
}

void NamedPipeRpcChannel::Close() {
  LONG was = InterlockedExchange(&is_connected_, Disconnected);
  if (was != Disconnected) {
    DWORD flags = 0;
    if (GetNamedPipeInfo(pipe_, &flags, NULL, NULL, NULL) != FALSE) {
      if (flags & PIPE_SERVER_END)
        DisconnectNamedPipe(pipe_);
    }
    CloseConnection();
  }
}

void NamedPipeRpcChannel::Send(void *message, size_t bytes) {
  if (is_connected_ != Connected)
    return;  // TODO: false;

  Overlapped *overlapped = AllocateOverlappedState(bytes + sizeof(uint32_t));

  assert(bytes < UINT32_MAX);
  *(uint32_t *)overlapped->buffer = bytes;
  memcpy(overlapped->buffer + sizeof(uint32_t), message, bytes);

  overlapped->operation_ = OverlappedOperation::Write;
  if (WriteFile(pipe_,
                overlapped->buffer,
                bytes + sizeof(uint32_t),
                NULL,
                overlapped) == FALSE) {
    // TODO: Handle ERROR_OPERATION_ABORTED when CancelIOEx implementation added
    // to disconnect.
    // This would not be a surprise disconnect though.

    if (GetLastError() == ERROR_BROKEN_PIPE) {
      FreeOverlappedState(overlapped);
      std::cout << "error: Pipe broken while attempting to write" << std::endl
                << std::flush;
      HandleSurpriseDisconnect();
      // TODO: return false;
    } else if (GetLastError() != ERROR_IO_PENDING) {
      FreeOverlappedState(overlapped);
      std::cout << "error: GetLastError() == " << GetLastError() << std::endl
                << std::flush;
      // TODO: return false;
    }
  }
}

bool NamedPipeRpcChannel::Receive(void *message, size_t *bytes) {
  std::unique_lock<std::mutex> lock(queue_lock_);
  if (messages_.empty())
    return false;

  if (bytes == nullptr)
    return true;

  size_t buffer_size = *bytes;
  *bytes = messages_.front().bytes;
  const InboundMessage &msg = messages_.front();

  if (message == nullptr || buffer_size < msg.bytes)
    return true;

  memcpy(message, msg.message->buffer, msg.bytes);
  FreeOverlappedState(msg.message);
  messages_.pop();
  return true;
}

NamedPipeRpcChannel::NamedPipeRpcChannel(
    const NamedPipeRpcChannelBuilder &builder)
    : builder_(builder),
      pipe_(nullptr),
      completion_port_(nullptr),
      completion_thread_id_(0),
      completion_thread_(nullptr),
      disconnected_callback_(nullptr),
      is_connected_(NotConnected) {}

void NamedPipeRpcChannel::StartRead(
    int expected_size,
    OverlappedOperation::Type requested_operation) {
  if (is_connected_ != Connected)
    return;  // TODO: false;

  Overlapped *overlapped = AllocateOverlappedState(expected_size);
  overlapped->operation_ = requested_operation;

  if (ReadFile(pipe_, overlapped->buffer, expected_size, NULL, overlapped) ==
      FALSE) {
    // TODO: Handle ERROR_OPERATION_ABORTED when CancelIOEx implementation added
    // to disconnect.
    // This would not be a surprise disconnect though.

    if (GetLastError() == ERROR_BROKEN_PIPE) {
      FreeOverlappedState(overlapped);
      std::cout << "error: Pipe broken while attempting to read\n";
      std::cout.flush();
      HandleSurpriseDisconnect();
    } else if (GetLastError() != ERROR_IO_PENDING) {
      FreeOverlappedState(overlapped);
      std::cout << "error: GetLastError() == " << GetLastError() << "\n";
      std::cout.flush();
    }
  }
}

void NamedPipeRpcChannel::ReadOperationCompleted(Overlapped *overlapped,
                                                 DWORD bytes_read) {
  // std::cout << "Read operation " << overlapped->operation_ << " completed\n";
  // std::cout.flush();

  if (overlapped->operation_ == OverlappedOperation::ReadPrefix) {
    assert(bytes_read == sizeof(uint32_t));
    // TODO: For sake of compatibility use protobuf IO to serialize the message
    // size.
    int expected_message_size = *(uint32_t *)overlapped->buffer;

    // Free resource as soon as we finished dealing with it.
    FreeOverlappedState(overlapped);
    StartRead(expected_message_size, OverlappedOperation::ReadMessage);
  } else if (overlapped->operation_ == OverlappedOperation::ReadMessage) {
    InboundMessage msg;
    msg.bytes = bytes_read;
    msg.message = overlapped;
    // *new* TODO: Check that bytes_read is same as expected_message_size

    // *old* Free resource as soon as we finished dealing with it.
    // *new* Do not do that as in new implementation will enqueue incoming
    // messages. Free overlapped just after Receive is called and data
    // delivered to the caller.
    // FreeOverlappedState(overlapped);

    // Start next read before we process the message, so we don't have to wait
    // for Receive to handle current message.
    // TODO: The only problem with this is that we still blocking the
    // IoCompletionThreadProc.
    // So we get a lockup if client (native) receives event and attempts to make
    // a synchronous call.
    // This precisely problem does not occur in the .NET channel, because thread
    // pool is used to handle I/O.
    StartRead(sizeof(uint32_t), OverlappedOperation::ReadPrefix);

    // TODO: Note that we pass local instance here,
    // so make sure we do not invoke any asyncronous operation downstream
    // with this message without copying it first.
    // Starting another read before Receive finished may also improve
    // performance in case
    // the client is multithreaded and uses the same channel for all threads.
    // *new* Enqueue the message.
    // Receive(message);
    {
      std::unique_lock<std::mutex> lock(queue_lock_);
      messages_.push(msg);
    }
  } else {
    assert(false);  // Unexpected operation
  }
}

void NamedPipeRpcChannel::WriteOperationCompleted(Overlapped *overlapped,
                                                  DWORD bytes_written) {
  // std::cout << "Write operation " << overlapped->operation_ << "
  // completed\n";
  // std::cout.flush();
  FreeOverlappedState(overlapped);
}

bool NamedPipeRpcChannel::ConnectInternal(HANDLE pipe) {
  pipe_ = pipe;

  if (InterlockedCompareExchange(&is_connected_, Connected, NotConnected) ==
      NotConnected) {
    completion_port_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (completion_port_ == NULL) {
      InterlockedExchange(&is_connected_, NotConnected);
      return false;
    }

    completion_thread_ = CreateThread(NULL,
                                      0,
                                      &IoCompletionThreadProcThunk,
                                      this,
                                      CREATE_SUSPENDED,
                                      &completion_thread_id_);
    if (completion_thread_ == NULL) {
      CloseHandle(completion_port_);
      InterlockedExchange(&is_connected_, NotConnected);
      return false;
    }

    if (CreateIoCompletionPort(pipe_, completion_port_, (ULONG_PTR)pipe_, 0) ==
        NULL) {
      CloseHandle(completion_port_);
      InterlockedExchange(&is_connected_, NotConnected);
      return false;
    }

    if (ResumeThread(completion_thread_) == (DWORD) - 1) {
      // This is fatal condition. We cannot restart it again, because the pipe
      // is already associated with
      // completion port, so, we cannot retry it.
      InterlockedExchange(&is_connected_, Disconnected);
      return false;
    }

    // We should not attempt to start asynchronous read here because the thread
    // that
    // called Start may exit and then overlapped operation would fail.
    // This is the case when connector callback is called and we create channel
    // directly from the callback.
    // StartRead( sizeof( __int32 ), OverlappedOperation::ReadPrefix );
  }

  return true;
}

int NamedPipeRpcChannel::IoCompletionThreadProc() {
  // See note in Start
  StartRead(sizeof(uint32_t), OverlappedOperation::ReadPrefix);

  while (is_connected_ == Connected) {
    DWORD bytes_transferred;
    ULONG_PTR key;
    Overlapped *overlapped = NULL;
    if (GetQueuedCompletionStatus(completion_port_,
                                  &bytes_transferred,
                                  &key,
                                  (OVERLAPPED **)&overlapped,
                                  INFINITE) == FALSE) {
      std::cout << "Overlapped operation failed: " << GetLastError() << "\n";

      // If overlapped is NULL then GetQueuedCompletionStatus failed because of
      // invalid parameter or completion_port_ handle was closed.
      // This would also happen if timeout parameter is not INFINITE (which is
      // not the case here).
      if (overlapped != NULL)
        FreeOverlappedState(overlapped);

      // TODO: We possibly need to handle ERROR_OPERATION_ABORTED when
      // CancelIOEx implementation added to disconnect.
      // This would not be a surprise disconnect though.
      if (GetLastError() == ERROR_BROKEN_PIPE) {
        std::cout << "Pipe broken\n";
        HandleSurpriseDisconnect();
        break;
      }
    } else {
      switch (overlapped->operation_) {
        case OverlappedOperation::ReadPrefix:
        case OverlappedOperation::ReadMessage:
          ReadOperationCompleted(overlapped, bytes_transferred);
          break;

        case OverlappedOperation::Write:
          WriteOperationCompleted(overlapped, bytes_transferred);
          break;

        default:
          assert(false);  // Unknown operation
          break;
      }
    }
  }

  return 0;
}

HANDLE NamedPipeRpcChannel::CloseConnection() {
  assert(completion_port_ != nullptr);
  assert(pipe_ != NULL);
  assert(completion_thread_ != NULL);

  // At this point we need to drain all pending operations from the
  // I/O completion port queue and ensure none new are queued.
  // We spin here until we see that all overlapped objects returned to the pool.

  int last_free_object_count = overlapped_pool_.GetFreeObjectsCount();
  int attempts = 0;

  while (overlapped_pool_.GetTotalObjectsCount() -
             overlapped_pool_.GetFreeObjectsCount() >
         0) {
    DWORD bytes_transferred;
    ULONG_PTR key;
    Overlapped *overlapped = nullptr;

    if (GetQueuedCompletionStatus(completion_port_,
                                  &bytes_transferred,
                                  &key,
                                  (OVERLAPPED **)&overlapped,
                                  0) == FALSE) {
      if (overlapped != nullptr) {
        FreeOverlappedState(overlapped);
      } else {
        // If the previous call failed, we wait a bit, then we check if any
        // objects
        // were freed since the last failure. If some objects were freed,
        // then we reset number of attempts and still keep trying.
        // If no object were freed, then we so few more attempts before bailing
        // out.
        Sleep(500);

        if (last_free_object_count < overlapped_pool_.GetFreeObjectsCount()) {
          attempts = 10;
          last_free_object_count = overlapped_pool_.GetFreeObjectsCount();
        } else {
          if (--attempts == 0) {
            int used = overlapped_pool_.GetTotalObjectsCount() -
                       overlapped_pool_.GetFreeObjectsCount();
            std::cout << "warning: Failed to release overlapped buffers. "
                      << used << " buffers are still appears to be used.\n";
            break;
          }
        }
      }
    }
  }

  // If HandleSurpriseDisconnect was called from the I/O completion thread,
  // then it will finish after the callback is called.
  // Otherwise, it would finish too, but we want to check if our assumptions
  // are correct.
  if (GetCurrentThreadId() != completion_thread_id_) {
    DWORD wait_result = WaitForSingleObject(completion_thread_, 2000);
    if (wait_result == WAIT_TIMEOUT) {
      // TODO: Spit debugger message instead
      std::cout << "warning: The I/O completion thread seems to hung.\n";
    }

    assert(wait_result == WAIT_TIMEOUT);
  }

  HANDLE pipe = pipe_;

  CloseHandle(completion_thread_);
  completion_thread_id_ = 0;
  completion_thread_ = nullptr;

  CloseHandle(pipe_);
  CloseHandle(completion_port_);
  completion_port_ = nullptr;
  pipe_ = nullptr;

  return pipe;
}

void NamedPipeRpcChannel::HandleSurpriseDisconnect() {
  // We can get here from multiple points, but only one should be allowed to do
  // the work.
  if (InterlockedCompareExchange(&is_connected_, Disconnected, Connected) ==
      Connected) {
    HANDLE pipe = CloseConnection();
    InvokeDisconnectedCallback(pipe);
  }
}

void NamedPipeRpcChannel::InvokeDisconnectedCallback(HANDLE pipe) {
  if (disconnected_callback_ != nullptr)
    disconnected_callback_->Invoke(pipe);
}

Overlapped *NamedPipeRpcChannel::AllocateOverlappedState(int size) {
  assert(size > 0);

  Overlapped *overlapped = overlapped_pool_.Allocate();
  overlapped->buffer = buffer_pool_.Allocate(size);

  assert(overlapped != nullptr);
  assert(overlapped->buffer != nullptr);

  return overlapped;
}

void NamedPipeRpcChannel::FreeOverlappedState(Overlapped *overlapped) {
  assert(overlapped != nullptr);
  if (overlapped->buffer != nullptr)
    buffer_pool_.Deallocate(overlapped->buffer);
  overlapped_pool_.Deallocate(overlapped);
}

}  // namespace
