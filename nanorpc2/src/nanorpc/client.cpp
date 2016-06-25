#include "nanorpc/nanorpc2.h"

#include <memory>
#include <mutex>

namespace nanorpc2 {

Client::Client(std::unique_ptr<ClientChannelInterface> channel)
    : channel_(std::move(channel)) {}

Client::~Client() {
  Disconnect();
  if (receive_thread_ != nullptr)
    receive_thread_->join();
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
  channel_->Disconnect();
}

void Client::Disconnect() {
  channel_->Disconnect();
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
      break;
    }

    result_pending_cv_.wait(lock);
  }
}

void Client::ReceiveThreadProc() {
  while (channel_->GetStatus() != ChannelStatus::NotConnected) {
    auto rdbuf = channel_->Read(sizeof(uint32_t));
    if (rdbuf == nullptr)
      continue;

    uint32_t message_size;
    if (!rdbuf->ReadAs(&message_size))
      continue;

    rdbuf = channel_->Read(message_size);
    if (rdbuf == nullptr)
      continue;

    std::unique_ptr<RpcMessage> response{ std::make_unique<RpcMessage>() };
    response->ParseFromArray(rdbuf->Read(message_size), message_size);

    {
      std::lock_guard<std::mutex> lock(pending_calls_mtx_);

      auto pending_call = pending_calls_.find(response->id());
      if (pending_call == pending_calls_.end())
        continue;

      pending_call->second = std::move(response);
    }

    result_pending_cv_.notify_all();
  }
}

void Client::LazyCreateReceiveThread() {
  if (receive_thread_ == nullptr) {
    receive_thread_ =
        std::make_unique<std::thread>(&Client::ReceiveThreadProc, this);
  }
}

bool Client::EnsureConnection() {
  // TODO: Check if we are disconnecting and do not proceed if we do.
  // TODO: Ensure channel is able to handle calling Connect concurrently.
  if (channel_->GetStatus() != ChannelStatus::Established) {
    if (!channel_->Connect())
      return false;
  }

  return true;
}

}  // namespace nanorpc2
