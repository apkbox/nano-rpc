#include "server_call_dispatcher.h"

#include <cassert>
#include <iostream>

#include "rpc_event_service.hpp"
#include "rpc_object_manager.hpp"
#include "rpc_service.hpp"

namespace nanorpc2 {

class ExternallyControlledLifetimeWrapper : public IRpcService {
public:
  ExternallyControlledLifetimeWrapper(IRpcService *object) : object_(object) {}

  virtual void CallMethod(const RpcCall &rpc_call, RpcResult *rpc_result) {
    object_->CallMethod(rpc_call, rpc_result);
  }

private:
  IRpcService *object_;
};

RpcCallDispatcher::RpcCallDispatcher() {
  RegisterService(RpcObjectManager::ServiceName,
                  new ExternallyControlledLifetimeWrapper(&object_manager_));
  RegisterService(RpcEventService::ServiceName,
                  new ExternallyControlledLifetimeWrapper(&event_service_));
}

void RpcCallDispatcher::RegisterService(const std::string &name,
                                        IRpcService *service) {
  assert(service != nullptr);
  object_manager_.RegisterService(name, service);
}

void RpcCallDispatcher::RegisterService(IRpcStub *stub) {
  assert(stub != nullptr);
  object_manager_.RegisterService(stub);
}

bool RpcCallDispatcher::Dispatch(const RpcMessage &request, RpcMessage *response) {
  assert(response != nullptr);
  if (response == nullptr)
    return false;

  // *new* this I would guess supposed to handle client reply from
  // previously sent event. I don't know why I put it here.
  if (request.has_result() && request.result().status() != RpcSucceeded) {
    // TODO: Dont know what to do in this case yet.
    // We were waiting for the message and seen the channel break and received
    // notification about it.
    // The best we can do is to notify the context (not there yet) about broken
    // connection.
    // Currently there are no context and the server does not differentiate
    // between clients and serve them all as one.
    // TODO: Probably we should delist the client from event manager.
    return false;
  }

  // TODO: Asynchronous calls, see below.
  // Here we have the option of handling incoming calls sequentually
  // (basically allowing single thread service) or making them asynchronously
  // thus making the service implementation responsible for handling
  // multithreaded calls.
  // This theoretically can be controlled by the attribute
  // AllowAsynchronousCalls that applies to class and method (in override
  // fashion to the default server behavior).
  // The default server behavior can also be specified as Syncrhonous or
  // Asynchronous.
  // For now make all calls synchronous.

  IRpcService *service = nullptr;
  response->set_id(request.id());

  // Check if the call relates to the singleton or transient object and
  // then try to find the appropriate service.
  //
  // The singleton objects are registered as services and identified by service
  // name. The singleton object's lifetime is controlled by the server.
  // The transient objects are registered as result of a method call and
  // identified by context ID.
  // The lifetime of a transient object controlled by the client.
  if (request.call().object_id() != 0) {
    // Try to find object that corresponds to the marshalled interface.
    service = object_manager_.GetInstance(request.call().object_id());
    if (service == nullptr) {
      // The requested service was not found. Reply with an error.
      response->mutable_result()->set_status(RpcUnknownInterface);
      response->mutable_result()->set_error_message(
          "Marshalled object does not exist (was object disposed?).");
    }
  } else {
    // Try to find the requested service.
    service = object_manager_.GetService(request.call().service().c_str());
    if (service == nullptr) {
      // The requested service was not found. Reply with an error.
      response->mutable_result()->set_status(RpcUnknownInterface);
      response->mutable_result()->set_error_message("Unknown interface");
    }
  }

  // If service was found - service the call, otherwise respond with an error.
  if (service != nullptr) {
    RpcResult rpc_result;

    // Call the requested method.
    service->CallMethod(request.call(), &rpc_result);
    // Whether to send reply message is controlled via user message
    // option. This options should returned as call metadata from CallMethod.
    // TODO: Implement async calls (those that do not send reply).
    response->mutable_result()->MergeFrom(rpc_result);
  } else {
    // If client does not expect result, there is no point of sending one,
    // even if it is an error message.
    // TODO: It possibly make sense to implement catch all
    // handler on the client that would handle all reply messages that
    // do not have pending calls. If that's the case, then expects_result
    // condition have to be removed. Also, it make sense control the behavior,
    // whether the server replies with an error message, even if the call does
    // not expect the result, through an option on the server.
    // *new* this always holds true, because when service not found,
    // response contains an error result.
    assert(response->has_result());
  }

  return true;
}

#if 0
void RpcCallDispatcher::SendEvent(RpcMessage &rpcMessage) {
  assert(rpcMessage.has_call());

  // Check that client subscribed for event notification.
  if (event_service_.HasInterface(rpcMessage.call().service())) {
    Send(rpcMessage);
  }
}
#endif

}  // namespace
