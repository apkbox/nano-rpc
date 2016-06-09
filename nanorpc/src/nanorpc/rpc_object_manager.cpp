#include "nanorpc/rpc_object_manager.hpp"

namespace nanorpc {

const char *const RpcObjectManager::ServiceName =
    "NanoRpc.ObjectManagerService";

RpcObjectManager::RpcObjectManager() : last_object_id_(0) {}

RpcObjectManager::~RpcObjectManager() {
  for (std::map<RpcObjectId, IRpcService *>::const_iterator iter =
           objects_.begin();
       iter != objects_.end(); iter++) {
    delete iter->second;
  }
}

void RpcObjectManager::RegisterService(const std::string &name, IRpcService *service) {
  services_[name] = RegisterInstance(service);
}

void RpcObjectManager::RegisterService(IRpcStub *stub) {
  assert(stub != nullptr);
  RegisterService(stub->GetInterfaceName(), stub);
}

// Currently implementation assumes that only unique instances are registered.
// So, it is wrong if method returns the same object accross multiple calls.
RpcObjectId RpcObjectManager::RegisterInstance(IRpcService *instance) {
  assert(instance != nullptr);
  objects_[++last_object_id_] = instance;
  return last_object_id_;
}

IRpcService *RpcObjectManager::GetService(const char *name) {
  assert(name != nullptr);

  if (name == nullptr)
    return nullptr;

  return GetService(std::string(name));
}

IRpcService *RpcObjectManager::GetService(const std::string &name) {
  std::map<std::string, RpcObjectId>::const_iterator iter =
      services_.find(name);
  if (iter == services_.end())
    return nullptr;

  return GetInstance(iter->second);
}

IRpcService *RpcObjectManager::GetInstance(RpcObjectId object_id) {
  if (object_id == 0)
    return nullptr;

  std::map<RpcObjectId, IRpcService *>::const_iterator iter =
      objects_.find(object_id);
  if (iter == objects_.end())
    return nullptr;

  return iter->second;
}

void RpcObjectManager::DeleteObject(RpcObjectId object_id) {
  if (object_id == 0)
    return;

  const auto iter = objects_.find(object_id);
  if (iter == objects_.end())
    return;

  delete iter->second;
  objects_.erase(iter);
}

void RpcObjectManager::CallMethod(const RpcCall &rpc_call,
                                  RpcResult *rpc_result) {
  rpc_result->set_status(RpcSucceeded);

  if (rpc_call.method() == "Delete") {
    RpcObject rpc_object;
    if (!rpc_object.ParseFromString(rpc_call.call_data())) {
      rpc_result->set_status(RpcInvalidCallParameter);
      rpc_result->set_error_message("Invalid call parameter.");
    } else {
      RpcObjectId object_id = rpc_object.object_id();
      DeleteObject(object_id);
    }
  } else {
    rpc_result->set_status(RpcUnknownMethod);
    rpc_result->set_error_message("Unknown method.");
  }
}

} // namespace
