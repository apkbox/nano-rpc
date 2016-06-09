#include "nanorpc\rpc_client.hpp"
#include "nanorpc\rpc_stub.hpp"
#include "nanorpc\rpc_object_manager.hpp"
#include "codegen_test.nanorpc.pb.h"

class RpcObjectManager : public nanorpc::IRpcObjectManager {
public:
  nanorpc::RpcObjectId RegisterInstance(nanorpc::IRpcService *instance) override {
    return 1;
  }
};

class RpcClient : public nanorpc::IRpcClient {
public:
  RpcClient(nanorpc::IRpcStub *stub) : stub_(stub) { }

  void Send(nanorpc::RpcMessage &rpcMessage) override {

  }

  void SendWithReply(nanorpc::RpcMessage &rpcMessage, nanorpc::RpcResult *result) override {
    stub_->CallMethod(rpcMessage.call(), result);
  }

private:
  nanorpc::IRpcStub *stub_;
};

class TestServiceIntefaceImpl : public codegen_test::TestServiceInteface {
public:
  void Method_V_V() override {}
  void Method_V_b(bool value) override {}
  void Method_V_i(int32_t value) override {}
  void Method_V_u(uint32_t value) override {}
  void Method_V_s(int32_t value) override {}
  void Method_V_I(int64_t value) override {}
  void Method_V_U(uint64_t value) override {}
  void Method_V_S(int64_t value) override {}
  void Method_V_f(float value) override {}
  void Method_V_d(double value) override {}
  void Method_V_E(codegen_test::EnumType value) override {}
  void Method_V_A(const std::string &value) override {}
  void Method_V_W(const std::string &value) override {}
  void Method_V_M(const codegen_test::StructType &value) override {}
  bool Method_b_V() override { return true; }
  int32_t Method_i_V() override { return 0; }
  uint32_t Method_u_V() override { return 0; }
  int32_t Method_s_V() override { return 0; }
  int64_t Method_I_V() override { return 0; }
  uint64_t Method_U_V() override { return 0; }
  int64_t Method_S_V() override { return 0; }
  float Method_f_V() override { return 3.14; }
  double Method_d_V() override { return 0; }
  codegen_test::EnumType Method_E_V() override { return codegen_test::EnumValue2; }
  void Method_A_V(std::string *out__) override { *out__ = "test"; }
  void Method_W_V(std::string *out__) override { *out__ = "test"; }
  void Method_M_V(codegen_test::StructType *out__) override { }
  void Method_V_biuIUsSfdEAWM(bool bool_value, int32_t int32_value, uint32_t uint32_value, int64_t int64_value, uint64_t uint64_value, 
    int32_t sint32_value, int64_t sint64_value, float float_value, double double_value, 
    codegen_test::EnumType enum_value, const std::string &string_value, 
    const std::string &wstring_value, const codegen_test::StructType &struct_value) override {

  }
  void AsyncMethod_V_V() override {

  }
};


int main() {
  TestServiceIntefaceImpl impl;
  RpcObjectManager object_manager;

  codegen_test::TestServiceInteface_Stub stub(&object_manager, &impl);
  RpcClient client(&stub);
  codegen_test::TestServiceInteface_Proxy proxy(&client);

  proxy.Method_V_A("test_string");
  float f = proxy.Method_f_V();

  return 0;
}
