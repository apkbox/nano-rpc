#include "nanorpc/nanorpc2.h"

#include <memory>
#include <mutex>

namespace nanorpc2 {

Client::Client(std::unique_ptr<ClientChannelInterface> channel)
    : channel_(std::move(channel)) {}

Client::~Client() {
  try {
    is_disposing = true;
    Disconnect();
    if (receive_thread_ != nullptr)
      receive_thread_->join();
  }
  catch(...) {
  }
}

bool Client::StartListening(ServiceInterface *event_interface) {
  return false;
}

bool Client::StartListening(const std::string &name,
                            ServiceInterface *event_interface) {
  return false;
}

void Client::StopListening(const std::string &name) {}

void Client::CreateEventThread(int concurrency) {}

bool Client::PumpEvents(bool *dropped) {
  return false;
}

bool Client::WaitForEvents() {
  return false;
}

bool Client::ConnectAndWait() {
  return false;
}

void Client::Shutdown() {
  is_shutting_down = true;
  channel_->Shutdown();
  FlushPendingCalls();
  channel_->Disconnect();
  is_shutting_down = false;
}

void Client::Disconnect() {
  is_shutting_down = true;
  channel_->Disconnect();
  AbortPendingCalls();
  is_shutting_down = false;
}

bool Client::CallMethod(const RpcCall &rpc_call, RpcResult *rpc_result) {
  if (!EnsureConnection())
    return false;

  auto message = channel_->CreateWriteBuffer();

  // TODO: This is not efficient as CopyFrom is essentially
  // a memcopy. This is the reason the old IRpcClient interface
  // was taking RpcMessage instead of RpcCall.
  // This also holds for the server side (for RPC result).
  // BTW, this could be quit expensive if call is big (e.g. includes array).
  RpcMessage call_message;
  call_message.mutable_call()->CopyFrom(rpc_call);
  call_message.set_id(++last_message_id_);

  auto message_size = call_message.ByteSize();
  *message->WriteAs<uint32_t>() = message_size;
  call_message.SerializeWithCachedSizesToArray(
      reinterpret_cast<pb::uint8 *>(message->Write(message_size)));
  channel_->Write(std::move(message));

  RpcMessage result_message;
  WaitForResult(call_message.id(), &result_message);

  // TODO: Here is another case of copying.
  *rpc_result = result_message.result();

  return true;
}

void Client::WaitForResult(uint32_t call_id, RpcMessage *result) {
  std::call_once(receive_thread_started_, &Client::LazyCreateReceiveThread,
                 this);

  pending_calls_[call_id] = nullptr;

  std::unique_lock<std::mutex> lock(pending_calls_mtx_);
  while (true) {
    auto pending_call = pending_calls_.find(call_id);
    if (pending_call->first == call_id && pending_call->second != nullptr) {
      *result = *pending_call->second.get();
      pending_calls_.erase(call_id);
      break;
    }

    result_pending_cv_.wait(lock);
  }
}

void Client::ReceiveThreadProc() {
  while (!is_disposing) {
    while (channel_->GetStatus() == ChannelStatus::Established)
      HandleIncomingMessage();

    while (channel_->GetStatus() != ChannelStatus::Established && !is_disposing)
      std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }
}

void Client::LazyCreateReceiveThread() {
  if (is_shutting_down)
    return;

  if (receive_thread_ == nullptr) {
    receive_thread_ =
        std::make_unique<std::thread>(&Client::ReceiveThreadProc, this);
  }
}

bool Client::EnsureConnection() {
  if (is_shutting_down)
    return false;
  // TODO: Ensure channel is able to handle calling Connect concurrently.
  if (channel_->GetStatus() != ChannelStatus::Established) {
    if (!channel_->Connect())
      return false;
  }

  return true;
}

void Client::FlushPendingCalls() {
  while (true) {
    {
      std::lock_guard<std::mutex> lock(pending_calls_mtx_);
      if (pending_calls_.size() == 0)
        break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }
}

void Client::AbortPendingCalls() {
  std::unique_lock<std::mutex> lock(pending_calls_mtx_);
  pending_calls_.clear();
  result_pending_cv_.notify_all();
}

// Read the message from channel and post either to event queue or
// pending calls.
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

  std::unique_ptr<RpcMessage> response{ std::make_unique<RpcMessage>() };
  response->ParseFromArray(rdbuf->Read(message_size), message_size);

  // TODO: Determine here whether the message is an event or reply.
  // Post events into event queue.
  {
    std::lock_guard<std::mutex> lock(pending_calls_mtx_);

    auto pending_call = pending_calls_.find(response->id());
    if (pending_call == pending_calls_.end())
      return false;

    pending_call->second = std::move(response);
  }

  result_pending_cv_.notify_all();

  return true;
}

}  // namespace nanorpc2
