#if !defined(NANORPC_SERVER_CALL_DISPATCHER_H__)
#define NANORPC_SERVER_CALL_DISPATCHER_H__

#include <map>
#include <string>
#include <set>

#include "rpc_event_service.hpp"
#include "rpc_object_manager.hpp"
#include "rpc_message_sender.hpp"
#include "rpc_service.hpp"
#include "rpc_stub.hpp"
#include "rpc_proto/rpc_types.pb.h"

namespace nanorpc2 {

// Implements the server side that receives client's calls and replies with
// result.
class RpcCallDispatcher {
public:
  RpcCallDispatcher();
  ~RpcCallDispatcher() {}

  void RegisterService(const std::string &name, IRpcService *service);
  void RegisterService(IRpcStub *stub);

  IRpcObjectManager *get_object_manager() { return &object_manager_; }

  bool Dispatch(const RpcMessage &request, RpcMessage *response);

  // Sends a message to the client.
  // This method is used to deliver events to the client, so the server does not
  // expect reply.
  // void SendEvent(RpcMessage &rpcMessage);

private:
  // This service keeps track of event subscriptions.
  // If client is interested in receiving events, it should
  // subscribe providing event interface name.
  RpcEventService event_service_;

  // This service tracks transient objects.
  RpcObjectManager object_manager_;
};

}  // namespace

#endif  // NANORPC_SERVER_CALL_DISPATCHER_H__
