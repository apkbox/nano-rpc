#include "nanorpc/object_manager.h"

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>

namespace nanorpc {

ObjectManager::ObjectManager() : last_object_id_(0) {}

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

}  // namespace nanorpc
