#include "nanorpc/nanorpc2.h"

namespace nanorpc2 {

ObjectManager::ObjectManager() : last_object_id_(0) {}

void ObjectManager::AddService(const std::string &name,
                               ServiceInterface *service) {
  assert(service != nullptr);
  if (service == nullptr)
    return;

  services_[name] = service;
}

void ObjectManager::RemoveService(const std::string &name) {
  services_.erase(name);
}

ServiceInterface *ObjectManager::GetService(const std::string &name) {
  const auto iter = services_.find(name);
  if (iter == services_.end())
    return nullptr;

  return iter->second;
}

RpcObjectId ObjectManager::AddObject(ServiceInterface *instance) {
  assert(instance != nullptr);
  if (instance == nullptr)
    return 0;

  objects_[++last_object_id_].reset(instance);
  return last_object_id_;
}

void ObjectManager::DeleteObject(RpcObjectId object_id) {
  if (object_id == 0)
    return;

  objects_.erase(object_id);
}

ServiceInterface *ObjectManager::GetObject(RpcObjectId object_id) {
  if (object_id == 0)
    return nullptr;

  const auto iter = objects_.find(object_id);
  if (iter == objects_.end())
    return nullptr;

  return iter->second.get();
}

}  // namespace nanorpc2
