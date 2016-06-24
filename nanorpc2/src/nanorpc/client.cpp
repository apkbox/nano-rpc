#include "nanorpc/nanorpc2.h"

namespace nanorpc2 {

Client::Client(std::unique_ptr<ClientChannelInterface> channel)
    : channel_(std::move(channel)), last_message_id_(0) {}

bool Client::CallMethod(const RpcCall &rpc_call, RpcResult *rpc_result) {
  if (channel_->GetStatus() != ChannelStatus::Established) {
    if (!channel_->Connect())
      return false;
  }

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
  call_message.SerializeWithCachedSizesToArray(
      reinterpret_cast<pb::uint8 *>(message->Write(message_size)));
  channel_->Write(std::move(message));

  return true;
}

}  // namespace nanorpc2
