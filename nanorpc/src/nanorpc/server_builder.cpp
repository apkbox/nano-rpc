#include "nanorpc/server_builder.h"

#include "nanorpc/rpc_event_service.hpp"
#include "nanorpc/rpc_object_manager.hpp"
#include "nanorpc/rpc_server.h"

namespace nanorpc {

void ServerBuilder::RegisterService(const std::string &name, IRpcService *service) {
  services_.emplace_back(name, service);
}

RpcServer *ServerBuilder::Build() {
  RpcServer *server = new RpcServer();
  server->set_channel(channel_builder_->BuildServerChannel());

  for(auto entry : services_) {
    server->RegisterService(entry.name, entry.service);
  }

  return server;
}

}  // namespace
