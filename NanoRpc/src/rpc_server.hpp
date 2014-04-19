#if !defined(NANO_RPC_RPC_SERVER_HPP__)
#define NANO_RPC_RPC_SERVER_HPP__

#include <map>
#include <set>

#include "rpc_event_service.hpp"
#include "rpc_object_manager.hpp"
#include "rpc_message_sender.hpp"
#include "rpc_service.hpp"
#include "rpc_stub.hpp"
#include "RpcMessageTypes.pb.h"

namespace NanoRpc {

class RpcChannel;
class RpcServer;
class RpcClient;
class RpcController;

// Implements the server side that receives client's calls and replies with
// result.
class RpcServer : public IRpcMessageSender {
public:
  explicit RpcServer(RpcController *controller);
  ~RpcServer();

  void RegisterService(const char *name, IRpcService *service);
  void RegisterService(IRpcStub *stub);

  IRpcObjectManager *GetObjectManager() { return &object_manager_; }

  void Receive(const RpcMessage &rpcMessage);

  // Sends a message to the client.
  // This method is used to deliver events to the client, so the server does not
  // expect reply.
  virtual void Send(RpcMessage &rpcMessage);

private:
  RpcController *controller_;

  // This service keeps track of event subscriptions.
  // If client is interested in receiving events, it should
  // subscribe providing event interface name.
  RpcEventService event_service_;

  RpcObjectManager object_manager_;
};

} // namespace

#endif // NANO_RPC_RPC_SERVER_HPP__
