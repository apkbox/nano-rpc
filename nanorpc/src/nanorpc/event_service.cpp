#include "nanorpc/event_service.h"

namespace nanorpc {

const std::string EventService::kServiceName{"NanoRpc.RpcEventService"};

const std::string &EventService::GetInterfaceName() const {
  return kServiceName;
}

// Because the implementation is very simple, the stub and object are
// implemented in a single class.
bool EventService::CallMethod(const RpcCall &rpc_call, RpcResult *rpc_result) {
  RpcEvent rpc_event;
  bool parsed_ok = rpc_event.ParseFromString(rpc_call.call_data());
  if (!parsed_ok || rpc_event.event_name().empty()) {
    rpc_result->set_status(RpcInvalidCallParameter);
    rpc_result->set_error_message("Invalid call parameter.");
    return true;
  }

  if (rpc_call.method() == "Add") {
    Add(rpc_event.event_name());
  } else if (rpc_call.method() == "Remove") {
    Remove(rpc_event.event_name());
  }

  return true;
}

void EventService::Add(const std::string &event_interface_name) {
  event_interfaces_.insert(event_interface_name);
}

void EventService::Remove(const std::string &event_interface_name) {
  event_interfaces_.erase(event_interface_name);
}

bool EventService::HasInterface(const std::string &event_interface_name) {
  return event_interfaces_.find(event_interface_name) !=
         event_interfaces_.end();
}

const std::string RpcEventFilter::kServiceName{"NanoRpc.RpcEventFilter"};

const std::string &RpcEventFilter::GetInterfaceName() const {
  return kServiceName;
}

}  // namespace nanorpc
