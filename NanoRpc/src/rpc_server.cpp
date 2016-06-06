#include "rpc_server.h"

#include <cassert>
#include <string>
#include <iostream>

#include "rpc_service.hpp"

namespace nanorpc2 {

void RpcServer::RegisterService(const std::string &name, IRpcService *service) {
  call_dispatcher_.RegisterService(name, service);
}

bool RpcServer::Start() {
  if (!channel_->Connect())
    return false;

  while (true /* TODO: channel_->get_state() == Connected */) {
    size_t bytes = 0;
    if (channel_->Receive(nullptr, &bytes)) {
      char *buffer = buffer_pool_.Allocate(bytes);
      channel_->Receive(buffer, &bytes);
      RpcMessage request;
      RpcMessage response;
      request.ParseFromArray(buffer, bytes);
      buffer_pool_.Deallocate(buffer);
      if (call_dispatcher_.Dispatch(request, &response)) {
        buffer = buffer_pool_.Allocate(response.ByteSize());
        channel_->Send(buffer, response.ByteSize());
        buffer_pool_.Deallocate(buffer);
      }
    }
  }

  return true;
}

void RpcServer::Shutdown() {
}

} // namespace
