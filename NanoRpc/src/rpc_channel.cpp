
#include "rpc_channel.hpp"
#include "rpc_controller.hpp"

namespace NanoRpc {

RpcChannel::RpcChannel(RpcController *controller) : controller_(controller) {
  controller_->set_channel(this);
}

RpcChannel::~RpcChannel() {
  if (controller_ != NULL)
    controller_->set_channel(NULL);
  controller_ = NULL;
}

void RpcChannel::Receive(const RpcMessage &message) {
  controller_->Receive(message);
}

} // namespace
