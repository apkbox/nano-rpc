#include "nanorpc/client.h"

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

namespace nanorpc {

Client::Client(std::unique_ptr<ClientChannelInterface> channel)
    : channel_(std::move(channel)) {}

Client::~Client() {
  try {
    is_disposing = true;
    Disconnect();
    if (receive_thread_ != nullptr)
      receive_thread_->join();
  } catch (...) {
  }
}

bool Client::StartListening(ServiceInterface *event_interface) {
  return StartListening(event_interface->GetInterfaceName(), event_interface);
}

bool Client::StartListening(const std::string &name,
                            ServiceInterface *event_interface) {
  if (!this->EnsureConnection())
    return false;

  if (event_service_manager_.GetService(name) == nullptr) {
    RpcCall call_message;
    call_message.set_service("NanoRpc.RpcEventService");
    call_message.set_method("Add");
    call_message.set_object_id(0);  // not really needed as it defaults to zero

    RpcEvent event_message;
    event_message.set_event_name(name);
    event_message.SerializeToString(call_message.mutable_call_data());

    SendCallRequest(0, call_message);
    event_service_manager_.AddService(name, event_interface);
  }

  return true;
}

void Client::StopListening(const std::string &name) {
  if (channel_->GetStatus() == ChannelStatus::Established) {
    RpcCall call_message;
    call_message.set_service("NanoRpc.RpcEventService");
    call_message.set_method("Remove");
    call_message.set_object_id(0);  // not really needed as it defaults to zero

    RpcEvent event_message;
    event_message.set_event_name(name);
    event_message.SerializeToString(call_message.mutable_call_data());
    SendCallRequest(0, call_message);
  }

  event_service_manager_.RemoveService(name);

  // Discard this interface events from the event queue.
  {
    std::lock_guard<std::mutex> lock(event_queue_mtx_);
    std::remove_if(event_queue_.begin(), event_queue_.end(),
                   [&name](const EventCall &event_call) {
                     return name == event_call.call->call().service();
                   });
  }
}

void Client::CreateEventThread(int concurrency) {}

bool Client::PumpEvents(bool *dropped) {
  const bool kLowLatencyImpl = false;

  ServiceInterface *handler;
  std::unique_ptr<RpcMessage> event_message;
  std::unique_ptr<RpcMessage> event_message2;
  bool events_pending = false;
  {
    std::lock_guard<std::mutex> lock(event_queue_mtx_);
    if (dropped != nullptr)
      *dropped = event_dropped_;
    event_dropped_ = false;
    if (event_queue_.empty())
      return false;

    EventCall &event_call = event_queue_.front();
    event_message = std::move(event_call.call);
    handler = event_call.handler;
    event_queue_.pop_front();
    if (!kLowLatencyImpl)
      events_pending = event_queue_.size() > 0;
  }

  handler->CallMethod(event_message->call(), nullptr);

  if (kLowLatencyImpl) {
    std::lock_guard<std::mutex> lock(event_queue_mtx_);
    return !event_queue_.empty();
  } else {
    return events_pending;
  }
}

bool Client::WaitForEvents() {
  assert(false);
  std::unique_lock<std::mutex> lock(event_queue_mtx_);
  while (channel_->GetStatus() == ChannelStatus::Established) {
    event_pending_cv_.wait_for(lock, std::chrono::milliseconds(500));
    if (!event_queue_.empty())
      return true;
  }

  return false;
}

bool Client::ConnectAndWait() {
  return false;
}

void Client::Shutdown() {
  channel_->Shutdown();
  FlushPendingCalls();
  channel_->Disconnect();
}

void Client::Disconnect() {
  channel_->Disconnect();
  AbortPendingCalls();
}

bool Client::CallMethod(const RpcCall &rpc_call, RpcResult *rpc_result) {
  if (!EnsureConnection())
    return false;

  auto call_id = ++last_message_id_;
  SendCallRequest(call_id, rpc_call);

  RpcMessage result_message;
  WaitForResult(call_id, &result_message);

  // TODO: Here is another case of copying.
  *rpc_result = result_message.result();

  return true;
}

void Client::SendCallRequest(uint32_t id, const RpcCall &rpc_call) {
  auto message_buffer = channel_->CreateWriteBuffer();

  // TODO: This is not efficient as CopyFrom is essentially
  // a memcopy. This is the reason the old IRpcClient interface
  // was taking RpcMessage instead of RpcCall.
  // This also holds for the server side (for RPC result).
  // BTW, this could be quit expensive if call is big (e.g. includes array).
  RpcMessage call_message;
  call_message.set_id(id);
  call_message.mutable_call()->CopyFrom(rpc_call);

  auto message_size = call_message.ByteSize();
  *message_buffer->WriteAs<uint32_t>() = message_size;
  auto ptr = message_buffer->WriteAsArray<pb::uint8>(message_size);
  call_message.SerializeWithCachedSizesToArray(ptr);

  channel_->Write(std::move(message_buffer));
}

// Waits until result is received or call is cancelled.
// This method creates receiver thread if needed.
// Returns true if call result was successfully received.
bool Client::WaitForResult(uint32_t call_id, RpcMessage *result) {
  std::call_once(receive_thread_started_, &Client::LazyCreateReceiveThread,
                 this);

  std::unique_lock<std::mutex> lock(pending_calls_mtx_);
  pending_calls_[call_id].status = PendingCallStatus::WaitingResult;

  while (true) {
    const auto &pending_call = pending_calls_.find(call_id);
    if (pending_call != pending_calls_.end()) {
      if (pending_call->second.status == PendingCallStatus::Received) {
        *result = *pending_call->second.result.get();
        pending_calls_.erase(call_id);
        break;
      } else if (pending_call->second.status == PendingCallStatus::Cancelled) {
        pending_calls_.erase(call_id);
        return false;
      }
    }

    result_pending_cv_.wait(lock);
  }

  return true;
}

void Client::ReceiveThreadProc() {
  // If channel disconnected - stop processing messages until it comes back.
  while (!is_disposing) {
    while (channel_->GetStatus() == ChannelStatus::Established)
      HandleIncomingMessage();

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }
}

void Client::LazyCreateReceiveThread() {
  if (receive_thread_ == nullptr) {
    receive_thread_ =
        std::make_unique<std::thread>(&Client::ReceiveThreadProc, this);
  }
}

bool Client::EnsureConnection() {
  // TODO: Ensure channel is able to handle calling Connect concurrently.
  if (channel_->GetStatus() == ChannelStatus::Established)
    return true;

  return channel_->Connect();
}

// Waits for all pending calls to complete.
// This method assumes there are no new calls made.
void Client::FlushPendingCalls() {
  std::unique_lock<std::mutex> lock(pending_calls_mtx_);
  while (true) {
    if (pending_calls_.size() == 0)
      break;

    // Graceful shutdown keeps channel open until all calls complete
    // (but prevents sending new ones).
    // However, if channel closes, e.g. Disconnect called from other thread
    // while Shutdown is blocked, we want to exit, because the following wait
    // will never be satisfied by HandleIncomingMessage and outstanding calls,
    // that not yet dispatched, will not complete.
    if (channel_->GetStatus() != ChannelStatus::Established)
      break;

    result_pending_cv_.wait_for(lock, std::chrono::milliseconds(500));
  }
}

// Aborts all pending calls.
// This method assumes there are no new calls made.
void Client::AbortPendingCalls() {
  std::unique_lock<std::mutex> lock(pending_calls_mtx_);
  pending_calls_.clear();
  result_pending_cv_.notify_all();
}

// Reads the message from channel and posts either to event queue or pending
// calls.
// Returns true if message was successfully received, decoded and the recipient
// determined.
bool Client::HandleIncomingMessage() {
  auto rdbuf = channel_->Read(sizeof(uint32_t));
  if (rdbuf == nullptr)
    return false;

  uint32_t message_size;
  if (!rdbuf->ReadAs(&message_size))
    return false;

  rdbuf = channel_->Read(message_size);
  if (rdbuf == nullptr)
    return false;

  auto response = std::unique_ptr<RpcMessage>{std::make_unique<RpcMessage>()};
  response->ParseFromArray(rdbuf->Read(message_size), message_size);

  // If call portion of the message is filled, then it is an event.
  if (response->has_call()) {
    // Check first if anyone expects the event
    auto event_service = event_service_manager_.GetService(response->call().service());
    if (event_service != nullptr) {
      std::lock_guard<std::mutex> lock(event_queue_mtx_);
      // TODO: Implement queue size limiting. Drop the oldest events.
      // BUG: Although it looks smart to cache the handler
      // pointer to avoid extra lookup it is a bad move.
      // Between time we received event and user called
      // PumpEvent, the other thread coild have called StopListening
      // so, the pointer to the service will be invalid.
      // On the other hand StopListening should reliably remove
      // events that are not being listened.
      event_queue_.emplace_back(event_service, std::move(response));
      event_pending_cv_.notify_all();
    }
  } else {
    std::lock_guard<std::mutex> lock(pending_calls_mtx_);

    auto pending_call = pending_calls_.find(response->id());
    if (pending_call == pending_calls_.end())
      return false;

    pending_call->second.status = PendingCallStatus::Received;
    pending_call->second.result = std::move(response);

    result_pending_cv_.notify_all();
  }

  return true;
}

}  // namespace nanorpc
