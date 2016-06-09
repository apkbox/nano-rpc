#if !defined(NANO_RPC_RPC_SERVICE_HPP__)
#define NANO_RPC_RPC_SERVICE_HPP__

#include "nanorpc/rpc_types.pb.h"

namespace nanorpc {

class IRpcService {
public:
  virtual ~IRpcService() {}
  virtual void CallMethod(const RpcCall &rpc_call, RpcResult *rpc_result) = 0;
};

} // namespace

#endif // NANO_RPC_RPC_SERVICE_HPP__
