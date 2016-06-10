#if !defined(NANO_RPC_RPC_SERVER_HPP__)
#define NANO_RPC_RPC_SERVER_HPP__

#include <map>
#include <set>

#include "nanorpc/rpc_event_service.hpp"
#include "nanorpc/rpc_object_manager.hpp"
#include "nanorpc/rpc_message_sender.hpp"
#include "nanorpc/server_call_dispatcher.h"
#include "nanorpc/rpc_channel.h"
#include "nanorpc/rpc_service.hpp"
#include "nanorpc/rpc_stub.hpp"
#include "nanorpc/buffer_pool.hpp"
#include "nanorpc/rpc_types.pb.h"

namespace nanorpc {

class ServerBuilder;

class RpcServer {
public:
  ~RpcServer() {}

  void set_channel(RpcChannel *channel) {
    channel_.reset(channel);
  }

  void RegisterService(const std::string &name, IRpcService *service);

  bool Start();

  void Shutdown();

private:
  friend class ServerBuilder;
  RpcServer() {}

  std::unique_ptr<RpcChannel> channel_;
  RpcCallDispatcher call_dispatcher_;
  BufferPool buffer_pool_;
};

}  // namespace

#endif  // NANO_RPC_RPC_SERVER_HPP__
