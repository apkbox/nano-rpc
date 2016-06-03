#include "rpc_object_manager.hpp"

namespace NanoRpc {

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

void RpcObjectManager::RegisterService(const char *name, IRpcService *service) {
  assert(service != NULL);
  services_[name] = RegisterInstance(service);
}

void RpcObjectManager::RegisterService(IRpcStub *stub) {
  assert(stub != NULL);
  RegisterService(stub->GetInterfaceName(), stub);
}

// Currently implementation assumes that only unique instances are registered.
// So, it is wrong if method returns the same object accross multiple calls.
RpcObjectId RpcObjectManager::RegisterInstance(IRpcService *instance) {
  assert(instance != NULL);
  objects_[++last_object_id_] = instance;
  return last_object_id_;
}

IRpcService *RpcObjectManager::GetService(const char *name) {
  assert(name != NULL);

  if (name == NULL)
    return NULL;

  return GetService(std::string(name));
}

IRpcService *RpcObjectManager::GetService(const std::string &name) {
  std::map<std::string, RpcObjectId>::const_iterator iter =
      services_.find(name);
  if (iter == services_.end())
    return NULL;

  return GetInstance(iter->second);
}

IRpcService *RpcObjectManager::GetInstance(RpcObjectId object_id) {
  if (object_id == 0)
    return NULL;

  std::map<RpcObjectId, IRpcService *>::const_iterator iter =
      objects_.find(object_id);
  if (iter == objects_.end())
    return NULL;

  return iter->second;
}

void RpcObjectManager::DeleteObject(RpcObjectId object_id) {
  if (object_id == 0)
    return;

  std::map<RpcObjectId, IRpcService *>::const_iterator iter =
      objects_.find(object_id);
  if (iter == objects_.end())
    return;

  delete iter->second;
  objects_.erase(iter);
}

void RpcObjectManager::CallMethod(const RpcCall &rpc_call,
                                  RpcResult *rpc_result) {
  rpc_result->set_status(RpcSucceeded);

  if (rpc_call.method() == "Delete") {
    assert(rpc_call.parameters_size() == 1);

    if (rpc_call.parameters_size() != 1) {
      rpc_result->set_status(RpcInvalidCallParameter);
      rpc_result->set_error_message("Invalid call parameter.");
    } else {
      RpcObjectId object_id = rpc_call.parameters().Get(0).object_id_value();
      DeleteObject(object_id);
    }
  } else {
    rpc_result->set_status(RpcUnknownMethod);
    rpc_result->set_error_message("Unknown method.");
  }
}

} // namespace
