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
  TestServiceIntefaceImpl() : void_method_calls_(0)
  {
  }

  void Method_V_V() override { ++void_method_calls_; }
  void Method_V_b(bool value) override { b_ = value; }
  void Method_V_i(int32_t value) override { i_ = value; }
  void Method_V_u(uint32_t value) override { u_ = value; }
  void Method_V_s(int32_t value) override { s_ = value; }
  void Method_V_I(int64_t value) override { I_ = value; }
  void Method_V_U(uint64_t value) override { U_ = value; }
  void Method_V_S(int64_t value) override { S_ = value; }
  void Method_V_f(float value) override { f_ = value; }
  void Method_V_d(double value) override { d_ = value; }
  void Method_V_E(codegen_test::EnumType value) override { E_ = value; }
  void Method_V_A(const std::string &value) override { A_ = value; }
  void Method_V_W(const std::string &value) override { W_ = value; }
  void Method_V_M(const codegen_test::StructType &value) override { M_ = value; }
  bool Method_b_V() override { return b_; }
  int32_t Method_i_V() override { return i_; }
  uint32_t Method_u_V() override { return u_; }
  int32_t Method_s_V() override { return s_; }
  int64_t Method_I_V() override { return I_; }
  uint64_t Method_U_V() override { return U_; }
  int64_t Method_S_V() override { return S_; }
  float Method_f_V() override { return f_; }
  double Method_d_V() override { return d_; }
  codegen_test::EnumType Method_E_V() override { return E_; }
  void Method_A_V(std::string *out__) override { *out__ = A_; }
  void Method_W_V(std::string *out__) override { *out__ = W_; }
  void Method_M_V(codegen_test::StructType *out__) override { *out__ = M_; }
  void Method_V_biuIUsSfdEAWM(bool bool_value, int32_t int32_value, uint32_t uint32_value, int64_t int64_value, uint64_t uint64_value, 
    int32_t sint32_value, int64_t sint64_value, float float_value, double double_value, 
    codegen_test::EnumType enum_value, const std::string &string_value, 
    const std::string &wstring_value, const codegen_test::StructType &struct_value) override {

  }
  void AsyncMethod_V_V() override {
    ++void_method_calls_;
  }

  int calls() const { return void_method_calls_; }

private:
  int void_method_calls_;

  bool b_;
  int32_t i_;
  uint32_t u_;
  int32_t s_;
  int64_t I_;
  uint64_t U_;
  int64_t S_;
  float f_;
  double d_;
  codegen_test::EnumType E_;
  std::string A_;
  std::string W_;
  codegen_test::StructType M_;
};


int main() {
  TestServiceIntefaceImpl impl;
  RpcObjectManager object_manager;

  codegen_test::TestServiceInteface_Stub stub(&object_manager, &impl);
  RpcClient client(&stub);
  codegen_test::TestServiceInteface_Proxy proxy(&client);

  proxy.Method_V_V();
  proxy.Method_V_V();
  assert(impl.calls() == 2);

  proxy.AsyncMethod_V_V();
  proxy.AsyncMethod_V_V();
  assert(impl.calls() == 4);

  proxy.Method_V_b(true);
  assert(proxy.Method_b_V());

  proxy.Method_V_i(123);
  assert(proxy.Method_i_V() == 123);

  proxy.Method_V_u(234);
  assert(proxy.Method_u_V() == 234);

  proxy.Method_V_s(345);
  assert(proxy.Method_s_V() == 345);

  proxy.Method_V_I(567);
  assert(proxy.Method_I_V() == 567);

  proxy.Method_V_U(678);
  assert(proxy.Method_U_V() == 678);

  proxy.Method_V_S(891);
  assert(proxy.Method_S_V() == 891);

  proxy.Method_V_f(9.12f);
  assert(proxy.Method_f_V() == 9.12f);

  proxy.Method_V_d(1.23);
  assert(proxy.Method_d_V() == 1.23);

  proxy.Method_V_E(codegen_test::EnumValue2);
  assert(proxy.Method_E_V() == codegen_test::EnumValue2);

  std::string tmp;

  proxy.Method_V_A("345");
  proxy.Method_A_V(&tmp);
  assert(tmp == "345");

  proxy.Method_V_W("456");
  proxy.Method_W_V(&tmp);
  assert(tmp == "456");

  codegen_test::StructType in_value;
  codegen_test::StructType out_value;
  proxy.Method_V_M(in_value);
  proxy.Method_M_V(&out_value);
  assert(out_value.SerializeAsString() == in_value.SerializeAsString());

  return 0;
}
