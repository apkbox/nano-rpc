#if !defined(NANO_RPC_EVENT_SERVICE_HPP__)
#define NANO_RPC_EVENT_SERVICE_HPP__

#include <set>
#include <string>

#include "rpc_proto/rpc_types.pb.h"

#include "rpc_service.hpp"

namespace nanorpc2 {

// This service keeps track of event subscriptions.
// When client interested in receiving events through specific
// event interface it should send subscription message the the server.
class RpcEventService : public IRpcService {
public:
  static const char *const ServiceName;

  void CallMethod(const RpcCall &rpc_call, RpcResult *rpc_result);

  void Add(const std::string &event_interface_name);
  void Remove(const std::string &event_interface_name);

  bool HasInterface(const std::string &event_interface_name);

private:
  std::set<std::string> event_interfaces_;
};

}  // namespace

#endif  // NANO_RPC_EVENT_SERVICE_HPP__
