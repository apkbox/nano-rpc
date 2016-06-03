#if !defined(APP_INTERFACE_STUBS_INCLUDED__)
#define APP_INTERFACE_STUBS_INCLUDED__

#include "nano_rpc.hpp"
#include "app_interface.h"

namespace app {

class ITestApp_Stub : public NanoRpc::IRpcStub {
public:
  explicit ITestApp_Stub(NanoRpc::IRpcObjectManager *object_manager,
                         ITestApp *impl)
      : object_manager_(object_manager), impl_(impl) {}

  const char *GetInterfaceName() const;

  void CallMethod(const NanoRpc::RpcCall &rpc_call,
                  NanoRpc::RpcResult *rpc_result);

private:
  NanoRpc::IRpcObjectManager *object_manager_;
  ITestApp *impl_;
};

} // namespace

#endif // APP_INTERFACE_STUBS_INCLUDED__
