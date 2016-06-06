#if !defined(NANO_RPC_RPC_OBJECT_MANANGER_HPP__)
#define NANO_RPC_RPC_OBJECT_MANANGER_HPP__

#include <string>
#include <map>

#include "rpc_proto/rpc_types.pb.h"

#include "rpc_service.hpp"
#include "rpc_stub.hpp"

namespace nanorpc2 {

typedef unsigned int RpcObjectId;

class IRpcObjectManager {
public:
  virtual ~IRpcObjectManager() {}
  virtual RpcObjectId RegisterInstance(IRpcService *instance) = 0;
};

class RpcObjectManager : public IRpcService, public IRpcObjectManager {
public:
  static const char *const ServiceName;

  RpcObjectManager();
  virtual ~RpcObjectManager();

  void RegisterService(const std::string &name, IRpcService *service);
  void RegisterService(IRpcStub *stub);

  // Currently implementation assumes that only unique instances are registered.
  // So, it is wrong if method returns the same object accross multiple calls.
  RpcObjectId RegisterInstance(IRpcService *instance);

  IRpcService *GetService(const char *name);
  IRpcService *GetService(const std::string &name);
  IRpcService *GetInstance(RpcObjectId object_id);

  void DeleteObject(RpcObjectId object_id);

  void CallMethod(const RpcCall &rpc_call, RpcResult *rpc_result);

private:
  RpcObjectId last_object_id_;

  std::map<std::string, RpcObjectId> services_;
  std::map<RpcObjectId, IRpcService *> objects_;
};

} // namespace

#endif // NANO_RPC_RPC_OBJECT_MANANGER_HPP__
