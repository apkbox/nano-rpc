#if !defined(NANORPC_SERVER_BUILDER_H__)
#define NANORPC_SERVER_BUILDER_H__

#include <memory>
#include <vector>

#include "nanorpc/rpc_channel.h"
#include "nanorpc/rpc_service.hpp"
#include "nanorpc/rpc_server.h"

namespace nanorpc {

class ServerBuilder {
public:
  ServerBuilder() {}

  void set_channel_builder(ChannelBuilder *channel_builder) {
    channel_builder_ = channel_builder;
  }

  void RegisterService(const std::string &name, IRpcService *service);
  void RegisterService(IRpcStub *stub);

  RpcServer *Build();

private:
  struct ServiceEntry {
    std::string name;
    IRpcService *service;
    ServiceEntry() : service(nullptr) {}
    ServiceEntry(const std::string &name, IRpcService * service)
        : name(name), service(service) {}
  };

  ChannelBuilder *channel_builder_;
  std::vector<ServiceEntry> services_;
};

}  // namespace

#endif  // NANORPC_SERVER_BUILDER_H__
