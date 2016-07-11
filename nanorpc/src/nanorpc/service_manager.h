#if !defined(NANORPC_SERVICE_MANAGER_H__)
#define NANORPC_SERVICE_MANAGER_H__

#include <map>
#include <string>

#include "nanorpc/nanorpc2.h"

namespace nanorpc {

class ServiceInterface;

// TODO: Methods of this class should be thread-safe, because on the client
// these can be called concurrently from app threads while
// registering/unregistering event listeners and from incoming messages thread
// while looking up for event routing.
// TODO: Make it a template so we can specify locking policy.
// This is needed for per-context service manager as it does not require locking.
class ServiceManager {
public:
  ServiceManager() {}

  void AddService(const std::string &name, ServiceInterface *service);
  void RemoveService(const std::string &name);
  ServiceInterface *GetService(const std::string &name);

private:
  ServiceManager(const ServiceManager &) = delete;
  ServiceManager &operator=(const ServiceManager &) = delete;

  std::map<std::string, ServiceInterface *> services_;
};

}  // namespace nanorpc

#endif  // NANORPC_SERVICE_MANAGER_H__
