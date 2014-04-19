
#include "rpc_controller.hpp"
#include "rpc_channel.hpp"
#include "rpc_server.hpp"
#include "rpc_client.hpp"

namespace NanoRpc {

void RpcController::Send(const RpcMessage &message) { channel_->Send(message); }

void RpcController::Receive(const RpcMessage &message) {
  // TODO: Implement client handling
  if (message.has_result() && message.result().status() != RpcSucceeded) {
    if (server_ != NULL)
      server_->Receive(message);
  } else {
    if (message.has_call()) {
      if (server_ != NULL)
        server_->Receive(message);
    }
  }
}

} // namespace
