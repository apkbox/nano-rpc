#if !defined(NANORPC_OBJECT_MANAGER_H__)
#define NANORPC_OBJECT_MANAGER_H__

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "nanorpc/nanorpc2.h"

namespace nanorpc {

class ServiceInterface;

class ObjectManager : public ObjectManagerInterface {
public:
  ObjectManager();

  void AddService(const std::string &name, ServiceInterface *service);
  void RemoveService(const std::string &name);
  ServiceInterface *GetService(const std::string &name);

  // Currently implementation assumes that only unique instances are registered.
  // So, it is wrong if method returns the same object accross multiple calls.
  RpcObjectId AddObject(ServiceInterface *instance);
  void DeleteObject(RpcObjectId object_id);
  ServiceInterface *GetObject(RpcObjectId object_id);

private:
  ObjectManager(const ObjectManager &) = delete;
  ObjectManager &operator=(const ObjectManager &) = delete;

  RpcObjectId last_object_id_;

  std::map<std::string, ServiceInterface *> services_;
  std::unordered_map<RpcObjectId, std::unique_ptr<ServiceInterface>> objects_;
};

}  // namespace nanorpc

#endif  // NANORPC_OBJECT_MANAGER_H__
