// Generated by the nanorpc protobuf plugin.
// If you make any local change, they will be lost.
// source: codegen_test.proto

#include "codegen_test.nanorpc.pb.h"
#include "google/protobuf/wrappers.pb.h"

namespace code_gen_test {

const char *ITestServiceInteface_Stub::GetInterfaceName() const {
  return "code_gen_test.ITestServiceInteface";
}

void ITestServiceInteface_Stub::CallMethod(const nanorpc2::RpcCall &rpc_call, nanorpc2::RpcResult *rpc_result) {
  if (rpc_call.method() == "Method_V_V") {

    impl_->Method_V_V();

  } else if (rpc_call.method() == "Method_V_b") {
    google::protobuf::BoolValue in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    bool value = in_arg__.value();

    impl_->Method_V_b(value);

  } else if (rpc_call.method() == "Method_V_i") {
    google::protobuf::Int32Value in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    int32_t value = in_arg__.value();

    impl_->Method_V_i(value);

  } else if (rpc_call.method() == "Method_V_u") {
    google::protobuf::UInt32Value in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    uint32_t value = in_arg__.value();

    impl_->Method_V_u(value);

  } else if (rpc_call.method() == "Method_V_s") {
    nanorpc2::SInt32Value in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    int32_t value = in_arg__.value();

    impl_->Method_V_s(value);

  } else if (rpc_call.method() == "Method_V_I") {
    google::protobuf::Int64Value in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    int64_t value = in_arg__.value();

    impl_->Method_V_I(value);

  } else if (rpc_call.method() == "Method_V_U") {
    google::protobuf::UInt64Value in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    uint64_t value = in_arg__.value();

    impl_->Method_V_U(value);

  } else if (rpc_call.method() == "Method_V_S") {
    nanorpc2::SInt64Value in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    int64_t value = in_arg__.value();

    impl_->Method_V_S(value);

  } else if (rpc_call.method() == "Method_V_f") {
    google::protobuf::FloatValue in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    float value = in_arg__.value();

    impl_->Method_V_f(value);

  } else if (rpc_call.method() == "Method_V_d") {
    google::protobuf::DoubleValue in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    double value = in_arg__.value();

    impl_->Method_V_d(value);

  } else if (rpc_call.method() == "Method_V_E") {
    EnumType_wrapper__ in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    EnumType value = in_arg__.value();

    impl_->Method_V_E(value);

  } else if (rpc_call.method() == "Method_V_A") {
    google::protobuf::StringValue in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    const std::string &value = in_arg__.value();

    impl_->Method_V_A(value);

  } else if (rpc_call.method() == "Method_V_W") {
    nanorpc2::WideStringValue in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    const std::string &value = in_arg__.value();

    impl_->Method_V_W(value);

  } else if (rpc_call.method() == "Method_V_M") {
    StructType in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    const StructType &value = in_arg__;

    impl_->Method_V_M(value);

  } else if (rpc_call.method() == "Method_b_V") {

    bool out__;
    google::protobuf::BoolValue out_pb__;
    out__ = impl_->Method_b_V();

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
  } else if (rpc_call.method() == "Method_i_V") {

    int32_t out__;
    google::protobuf::Int32Value out_pb__;
    out__ = impl_->Method_i_V();

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
  } else if (rpc_call.method() == "Method_u_V") {

    uint32_t out__;
    google::protobuf::UInt32Value out_pb__;
    out__ = impl_->Method_u_V();

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
  } else if (rpc_call.method() == "Method_s_V") {

    int32_t out__;
    nanorpc2::SInt32Value out_pb__;
    out__ = impl_->Method_s_V();

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
  } else if (rpc_call.method() == "Method_I_V") {

    int64_t out__;
    google::protobuf::Int64Value out_pb__;
    out__ = impl_->Method_I_V();

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
  } else if (rpc_call.method() == "Method_U_V") {

    uint64_t out__;
    google::protobuf::UInt64Value out_pb__;
    out__ = impl_->Method_U_V();

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
  } else if (rpc_call.method() == "Method_S_V") {

    int64_t out__;
    nanorpc2::SInt64Value out_pb__;
    out__ = impl_->Method_S_V();

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
  } else if (rpc_call.method() == "Method_f_V") {

    float out__;
    google::protobuf::FloatValue out_pb__;
    out__ = impl_->Method_f_V();

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
  } else if (rpc_call.method() == "Method_d_V") {

    double out__;
    google::protobuf::DoubleValue out_pb__;
    out__ = impl_->Method_d_V();

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
  } else if (rpc_call.method() == "Method_E_V") {

    EnumType out__;
    EnumType_wrapper__ out_pb__;
    out__ = impl_->Method_E_V();

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
  } else if (rpc_call.method() == "Method_A_V") {

    std::string out__;
    google::protobuf::StringValue out_pb__;
    impl_->Method_A_V(&out__);

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
  } else if (rpc_call.method() == "Method_W_V") {

    std::string out__;
    nanorpc2::WideStringValue out_pb__;
    impl_->Method_W_V(&out__);

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
  } else if (rpc_call.method() == "Method_M_V") {

    StructType out__;
    impl_->Method_M_V(&out__);

    out__.SerializeToString(rpc_result->mutable_result_data());
  } else if (rpc_call.method() == "Method_V_biuIUsSfdEAWM") {
    Method_V_biuIUsSfdEAWM_args__ args__;
    args__.ParseFromString(rpc_call.call_data());

    bool bool_value = args__.bool_value();
    int32_t int32_value = args__.int32_value();
    uint32_t uint32_value = args__.uint32_value();
    int64_t int64_value = args__.int64_value();
    uint64_t uint64_value = args__.uint64_value();
    int32_t sint32_value = args__.sint32_value();
    int64_t sint64_value = args__.sint64_value();
    float float_value = args__.float_value();
    double double_value = args__.double_value();
    EnumType enum_value = args__.enum_value();
    const std::string &string_value = args__.string_value();
    const std::string &wstring_value = args__.wstring_value();
    const StructType &struct_value = args__.struct_value();


    impl_->Method_V_biuIUsSfdEAWM(bool_value, int32_value, uint32_value, int64_value, uint64_value, sint32_value, sint64_value, float_value, double_value, enum_value, string_value, wstring_value, struct_value);

  } else if (rpc_call.method() == "AsyncMethod_V_V") {

    impl_->AsyncMethod_V_V();

  }

}

}  // namespace code_gen_test


