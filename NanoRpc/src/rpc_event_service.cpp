#include "rpc_event_service.hpp"
#include "string_conversion.hpp"

namespace NanoRpc {

const char *const RpcEventService::ServiceName = "NanoRpc.RpcEventService";

// Because the implementation is very simple, the stub and object are
// implemented in
// a single class.
void RpcEventService::CallMethod(const RpcCall &rpc_call,
                                 RpcResult *rpc_result) {
  if (rpc_call.method() == "Add") {
    assert(rpc_call.parameters_size() == 1);
    assert(rpc_call.parameters().Get(0).has_string_value());

    if (rpc_call.parameters_size() != 1 ||
        !rpc_call.parameters().Get(0).has_string_value()) {
      rpc_result->set_status(RpcInvalidCallParameter);
      rpc_result->set_error_message("Invalid call parameter.");
    } else {
      const std::string &event_interface_name =
          rpc_call.parameters().Get(0).string_value();
      Add(event_interface_name);
    }
  } else if (rpc_call.method() == "Remove") {
    assert(rpc_call.parameters_size() == 1);
    assert(rpc_call.parameters().Get(0).has_string_value());

    if (rpc_call.parameters_size() != 1 ||
        !rpc_call.parameters().Get(0).has_string_value()) {
      rpc_result->set_status(RpcInvalidCallParameter);
      rpc_result->set_error_message("Invalid call parameter.");
    } else {
      const std::string &event_interface_name =
          rpc_call.parameters().Get(0).string_value();
      Remove(event_interface_name);
    }
  }
}

void RpcEventService::Add(const std::string &event_interface_name) {
  event_interfaces_.insert(event_interface_name);
}

void RpcEventService::Remove(const std::string &event_interface_name) {
  event_interfaces_.erase(event_interface_name);
}

bool RpcEventService::HasInterface(const std::string &event_interface_name) {
  return event_interfaces_.find(event_interface_name) !=
         event_interfaces_.end();
}

} // namespace
