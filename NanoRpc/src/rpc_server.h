#if !defined(NANO_RPC_RPC_SERVER_HPP__)
#define NANO_RPC_RPC_SERVER_HPP__

#include <map>
#include <set>

#include "rpc_event_service.hpp"
#include "rpc_object_manager.hpp"
#include "rpc_message_sender.hpp"
#include "server_call_dispatcher.h"
#include "rpc_channel.h"
#include "rpc_service.hpp"
#include "rpc_stub.hpp"
#include "buffer_pool.hpp"
#include "rpc_proto/rpc_types.pb.h"

namespace nanorpc2 {

class RpcServerBuilder;

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
