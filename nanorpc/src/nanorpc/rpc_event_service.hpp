#if !defined(NANO_RPC_EVENT_SERVICE_HPP__)
#define NANO_RPC_EVENT_SERVICE_HPP__

#include <set>
#include <string>

#include "nanorpc/nanorpc2.h"
#include "nanorpc/rpc_types.pb.h"

namespace nanorpc {

// This service keeps track of event subscriptions.
// When client interested in receiving events through specific
// event interface it should send subscription message the the server.
class RpcEventService : public ServiceInterface {
public:
  static const std::string kServiceName;

  const std::string &GetInterfaceName() const override;
  bool CallMethod(const RpcCall &rpc_call, RpcResult *rpc_result) override;

  void Add(const std::string &event_interface_name);
  void Remove(const std::string &event_interface_name);

  bool HasInterface(const std::string &event_interface_name);

private:
  std::set<std::string> event_interfaces_;
};

}  // namespace

#endif  // NANO_RPC_EVENT_SERVICE_HPP__
