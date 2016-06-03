#include "app_interface_stubs.h"

namespace app {

const char *ITestApp_Stub::GetInterfaceName() const { return "app.ITestApp"; }

void ITestApp_Stub::CallMethod(const NanoRpc::RpcCall &rpc_call,
                               NanoRpc::RpcResult *rpc_result) {
  if (rpc_call.method() == "get_BoolValue") {
    bool result;
    result = impl_->get_BoolValue();
    google::protobuf::BoolValue wrapper;
    wrapper.set_value(result);
    wrapper.SerializeToString(
        rpc_result->mutable_call_result()->mutable_value());
  } else if (rpc_call.method() == "set_BoolValue") {
    google::protobuf::BoolValue value;
    value.ParseFromString(rpc_call.parameters().Get(0).value());
    impl_->set_BoolValue(value.value());
  } else if (rpc_call.method() == "get_IntValue") {
    int result;
    result = impl_->get_IntValue();
    google::protobuf::Int32Value wrapper;
    wrapper.set_value(result);
    wrapper.SerializeToString(
        rpc_result->mutable_call_result()->mutable_value());
  } else if (rpc_call.method() == "set_IntValue") {
    google::protobuf::Int32Value value;
    value.ParseFromString(rpc_call.parameters().Get(0).value());
    impl_->set_IntValue(value.value());
  }
}

} // namespace
