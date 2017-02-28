#include "codegen_test.nanorpc.pb.h"

class RpcClient : public nanorpc::ServiceProxyInterface {
public:
  RpcClient(nanorpc::ServiceInterface *stub) : stub_(stub) { }

  bool CallMethod(const nanorpc::RpcCall &rpc_call, nanorpc::RpcResult *rpc_result) override {
    return stub_->CallMethod(rpc_call, rpc_result);
  }

private:
  nanorpc::ServiceInterface *stub_;
};

class TestServiceIntefaceImpl : public codegen_test::TestServiceInteface {
public:
  TestServiceIntefaceImpl() : void_method_calls_(0)
  {
  }

  void Method_V_V(nanorpc::ServerContextInterface *) override { ++void_method_calls_; }
  void Method_V_b(nanorpc::ServerContextInterface *, bool value) override { b_ = value; }
  void Method_V_i(nanorpc::ServerContextInterface *, int32_t value) override { i_ = value; }
  void Method_V_u(nanorpc::ServerContextInterface *, uint32_t value) override { u_ = value; }
  void Method_V_s(nanorpc::ServerContextInterface *, int32_t value) override { s_ = value; }
  void Method_V_I(nanorpc::ServerContextInterface *, int64_t value) override { I_ = value; }
  void Method_V_U(nanorpc::ServerContextInterface *, uint64_t value) override { U_ = value; }
  void Method_V_S(nanorpc::ServerContextInterface *, int64_t value) override { S_ = value; }
  void Method_V_f(nanorpc::ServerContextInterface *, float value) override { f_ = value; }
  void Method_V_d(nanorpc::ServerContextInterface *, double value) override { d_ = value; }
  void Method_V_E(nanorpc::ServerContextInterface *, codegen_test::EnumType value) override { E_ = value; }
  void Method_V_A(nanorpc::ServerContextInterface *, const std::string &value) override { A_ = value; }
  void Method_V_W(nanorpc::ServerContextInterface *, const std::string &value) override { W_ = value; }
  void Method_V_M(nanorpc::ServerContextInterface *, const codegen_test::StructType &value) override { M_ = value; }
  bool Method_b_V(nanorpc::ServerContextInterface *) override { return b_; }
  int32_t Method_i_V(nanorpc::ServerContextInterface *) override { return i_; }
  uint32_t Method_u_V(nanorpc::ServerContextInterface *) override { return u_; }
  int32_t Method_s_V(nanorpc::ServerContextInterface *) override { return s_; }
  int64_t Method_I_V(nanorpc::ServerContextInterface *) override { return I_; }
  uint64_t Method_U_V(nanorpc::ServerContextInterface *) override { return U_; }
  int64_t Method_S_V(nanorpc::ServerContextInterface *) override { return S_; }
  float Method_f_V(nanorpc::ServerContextInterface *) override { return f_; }
  double Method_d_V(nanorpc::ServerContextInterface *) override { return d_; }
  codegen_test::EnumType Method_E_V(nanorpc::ServerContextInterface *) override { return E_; }
  void Method_A_V(nanorpc::ServerContextInterface *, std::string *out__) override { *out__ = A_; }
  void Method_W_V(nanorpc::ServerContextInterface *, std::string *out__) override { *out__ = W_; }
  void Method_M_V(nanorpc::ServerContextInterface *, codegen_test::StructType *out__) override { *out__ = M_; }
  void Method_V_biuIUsSfdEAWM(nanorpc::ServerContextInterface *, bool bool_value, int32_t int32_value, uint32_t uint32_value, int64_t int64_value, uint64_t uint64_value, 
    int32_t sint32_value, int64_t sint64_value, float float_value, double double_value, 
    codegen_test::EnumType enum_value, const std::string &string_value, 
    const std::string &wstring_value, const codegen_test::StructType &struct_value) override {

  }
  void AsyncMethod_V_V(nanorpc::ServerContextInterface *) override {
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

  codegen_test::TestServiceInteface_Stub stub(&impl, nullptr);
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
