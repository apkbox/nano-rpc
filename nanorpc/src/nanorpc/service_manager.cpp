#include "nanorpc/service_manager.h"

#include <cassert>
#include <map>
#include <string>

namespace nanorpc {

void ServiceManager::AddService(const std::string &name,
                                ServiceInterface *service) {
  assert(service != nullptr);
  if (service == nullptr)
    return;

  services_[name] = service;
}

void ServiceManager::RemoveService(const std::string &name) {
  services_.erase(name);
}

ServiceInterface *ServiceManager::GetService(const std::string &name) {
  const auto iter = services_.find(name);
  if (iter == services_.end())
    return nullptr;

  return iter->second;
}

}  // namespace nanorpc
