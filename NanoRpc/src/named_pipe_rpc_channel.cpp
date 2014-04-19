// This implementation uses exclusively asynchronous I/O for the reason of
// properly handling surprise disconnect condition.

#include "named_pipe_rpc_channel.hpp"

#include <iostream>

#include <windows.h>

#include "rpc_controller.hpp"

namespace NanoRpc {

NamedPipeRpcChannel::NamedPipeRpcChannel(RpcController *controller,
                                         HANDLE pipe_handle)
    : RpcChannel(controller), pipe_(pipe_handle), completion_port_(NULL),
      completion_thread_id_(0), completion_thread_(NULL),
      disconnected_callback_(NULL), is_connected_(NotConnected) {}

NamedPipeRpcChannel::~NamedPipeRpcChannel() { Close(); }

bool NamedPipeRpcChannel::Start() {
  if (InterlockedCompareExchange(&is_connected_, Connected, NotConnected) ==
      NotConnected) {
    completion_port_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (completion_port_ == NULL) {
      InterlockedExchange(&is_connected_, NotConnected);
      return false;
    }

    completion_thread_ =
        CreateThread(NULL, 0, &IoCompletionThreadProcThunk, this,
                     CREATE_SUSPENDED, &completion_thread_id_);
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

void NamedPipeRpcChannel::Send(const RpcMessage &message) {
  if (is_connected_ != Connected)
    return; // TODO: false;

  Overlapped *overlapped =
      AllocateOverlappedState(message.ByteSize() + sizeof(__int32));

  *(__int32 *)overlapped->buffer = message.ByteSize();
  if (!message.SerializeToArray(overlapped->buffer + sizeof(__int32),
                                message.ByteSize()))
    assert(false); // TODO: return proper error to the upper level in release

  overlapped->operation_ = OverlappedOperation::Write;
  if (WriteFile(pipe_, overlapped->buffer, message.ByteSize() + sizeof(__int32),
                NULL, overlapped) == FALSE) {
    // TODO: Handle ERROR_OPERATION_ABORTED when CancelIOEx implementation added
    // to disconnect.
    // This would not be a surprise disconnect though.

    if (GetLastError() == ERROR_BROKEN_PIPE) {
      FreeOverlappedState(overlapped);
      std::cout << "error: Pipe broken while attempting to write\n";
      std::cout.flush();
      HandleSurpriseDisconnect();
      // TODO: return false;
    } else if (GetLastError() != ERROR_IO_PENDING) {
      FreeOverlappedState(overlapped);
      std::cout << "error: GetLastError() == " << GetLastError() << "\n";
      std::cout.flush();
      // TODO: return false;
    }
  }
}

void
NamedPipeRpcChannel::StartRead(int expected_size,
                               OverlappedOperation::Type requested_operation) {
  if (is_connected_ != Connected)
    return; // TODO: false;

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
    assert(bytes_read == sizeof(__int32));
    // TODO: For sake of compatibility use protobuf IO to serialize the message
    // size.
    int expected_message_size = *(__int32 *)overlapped->buffer;

    // Free resource as soon as we finished dealing with it.
    FreeOverlappedState(overlapped);
    StartRead(expected_message_size, OverlappedOperation::ReadMessage);
  } else if (overlapped->operation_ == OverlappedOperation::ReadMessage) {
    RpcMessage message;
    if (!message.ParseFromArray(overlapped->buffer, bytes_read))
      assert(false); // TODO: Handle error in release

    // Free resource as soon as we finished dealing with it.
    FreeOverlappedState(overlapped);

    // Start next read before we process the message, so we don't have to wait
    // for Receive to handle current message.
    // TODO: Thie only problem with this is that we still blocking the
    // IoCompletionThreadProc.
    // So we get a lockup if client (native) receives event and attempts to make
    // a synchronous call.
    // This precisely problem does not occur in the .NET channel, because thread
    // pool is used to handle I/O.
    StartRead(sizeof(__int32), OverlappedOperation::ReadPrefix);

    // TODO: Note that we pass local instance here,
    // so make sure we do not invoke any asyncronous operation downstream
    // with this message without copying it first.
    // Starting another read before Receive finished may also improve
    // performance in case
    // the client is multithreaded and uses the same channel for all threads.
    Receive(message);
  } else {
    assert(false); // Unexpected operation
  }
}

void NamedPipeRpcChannel::WriteOperationCompleted(Overlapped *overlapped,
                                                  DWORD bytes_written) {
  // std::cout << "Write operation " << overlapped->operation_ << "
  // completed\n";
  // std::cout.flush();
  FreeOverlappedState(overlapped);
}

int NamedPipeRpcChannel::IoCompletionThreadProc() {
  // See note in Start
  StartRead(sizeof(__int32), OverlappedOperation::ReadPrefix);

  while (is_connected_ == Connected) {
    DWORD bytes_transferred;
    ULONG_PTR key;
    Overlapped *overlapped = NULL;
    if (GetQueuedCompletionStatus(completion_port_, &bytes_transferred, &key,
                                  (OVERLAPPED **)&overlapped,
                                  INFINITE) == FALSE) {
      std::cout << "Overlapped operation failed: " << GetLastError() << "\n";

      // If overlapped is NULL then GetQueuedCompletionStatus failed because of
      // invalid
      // parameter or completion_port_ handle was closed.
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
        assert(false); // Unknown operation
        break;
      }
    }
  }

  return 0;
}

HANDLE NamedPipeRpcChannel::CloseConnection() {
  assert(completion_port_ != NULL);
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
    Overlapped *overlapped = NULL;

    if (GetQueuedCompletionStatus(completion_port_, &bytes_transferred, &key,
                                  (OVERLAPPED **)&overlapped, 0) == FALSE) {
      if (overlapped != NULL) {
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
  completion_thread_ = NULL;

  CloseHandle(pipe_);
  CloseHandle(completion_port_);
  completion_port_ = NULL;
  pipe_ = NULL;

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
  if (disconnected_callback_ != NULL)
    disconnected_callback_->Invoke(pipe);
}

Overlapped *NamedPipeRpcChannel::AllocateOverlappedState(int size) {
  assert(size > 0);

  Overlapped *overlapped = overlapped_pool_.Allocate();
  overlapped->buffer = buffer_pool_.Allocate(size);

  assert(overlapped != NULL);
  assert(overlapped->buffer != NULL);

  return overlapped;
}

void NamedPipeRpcChannel::FreeOverlappedState(Overlapped *overlapped) {
  assert(overlapped != NULL);
  if (overlapped->buffer != NULL)
    buffer_pool_.Deallocate(overlapped->buffer);
  overlapped_pool_.Deallocate(overlapped);
}

} // namespace
