// Generated by the nanorpc protobuf plugin.
// If you make any local change, they will be lost.
// source: arizona_api_server.proto

#include "arizona_api_server.nanorpc.pb.h"
#include "google/protobuf/wrappers.pb.h"

namespace arizona_api_service {

const std::string SoftwareUpdateEvents_Stub::kServiceName("arizona_api_service.SoftwareUpdateEvents");

const std::string &SoftwareUpdateEvents_Stub::GetInterfaceName() const {
  return kServiceName;
}

bool SoftwareUpdateEvents_Stub::CallMethod(const nanorpc::RpcCall &rpc_call, nanorpc::RpcResult *rpc_result) {
  if (rpc_call.method() == "PackageListChanged") {
    impl_->PackageListChanged();
    return true;
  }

  // TODO: Here should be unknown method error stored into rpc_result.
  // TODO: Also an exception (code above must be guarded) result.

  return true;
}

const std::string ProductInfoInterface_Stub::kServiceName("arizona_api_service.ProductInfoInterface");

const std::string &ProductInfoInterface_Stub::GetInterfaceName() const {
  return kServiceName;
}

bool ProductInfoInterface_Stub::CallMethod(const nanorpc::RpcCall &rpc_call, nanorpc::RpcResult *rpc_result) {
  if (rpc_call.method() == "GetProductInfo") {
    ProductInfo out__;
    impl_->GetProductInfo(&out__);

    out__.SerializeToString(rpc_result->mutable_result_data());
    return true;
  }

  // TODO: Here should be unknown method error stored into rpc_result.
  // TODO: Also an exception (code above must be guarded) result.

  return true;
}

const std::string SoftwareUpdateInterface_Stub::kServiceName("arizona_api_service.SoftwareUpdateInterface");

const std::string &SoftwareUpdateInterface_Stub::GetInterfaceName() const {
  return kServiceName;
}

bool SoftwareUpdateInterface_Stub::CallMethod(const nanorpc::RpcCall &rpc_call, nanorpc::RpcResult *rpc_result) {
  if (rpc_call.method() == "GetAvailablePackages") {
    SoftwarePackageList out__;
    impl_->GetAvailablePackages(&out__);

    out__.SerializeToString(rpc_result->mutable_result_data());
    return true;
  } else if (rpc_call.method() == "StartPackageInstallation") {
    google::protobuf::StringValue in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    const std::string &value = in_arg__.value();

    bool out__;
    google::protobuf::BoolValue out_pb__;
    out__ = impl_->StartPackageInstallation(value);

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
    return true;
  } else if (rpc_call.method() == "DeletePackage") {
    google::protobuf::StringValue in_arg__;

    in_arg__.ParseFromString(rpc_call.call_data());
    const std::string &value = in_arg__.value();

    bool out__;
    google::protobuf::BoolValue out_pb__;
    out__ = impl_->DeletePackage(value);

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
    return true;
  } else if (rpc_call.method() == "GetPackageStorePath") {
    std::string out__;
    google::protobuf::StringValue out_pb__;
    impl_->GetPackageStorePath(&out__);

    out_pb__.set_value(out__);
    out_pb__.SerializeToString(rpc_result->mutable_result_data());
    return true;
  }

  // TODO: Here should be unknown method error stored into rpc_result.
  // TODO: Also an exception (code above must be guarded) result.

  return true;
}

void SoftwareUpdateEvents_EventProxy::PackageListChanged() {
  nanorpc::RpcCall rpc_call__;
  rpc_call__.set_service("arizona_api_service.SoftwareUpdateEvents");
  rpc_call__.set_method("PackageListChanged");
  event_source_->SendEvent(rpc_call__);
}

ProductInfoInterface_Proxy::~ProductInfoInterface_Proxy() {
  if (object_id_ != 0) {
    try {
      nanorpc::RpcCall rpc_call;
      rpc_call.set_service("NanoRpc.ObjectManagerService");
      rpc_call.set_method("Delete");
      nanorpc::RpcObject rpc_object;
      rpc_object.set_object_id(object_id_);
      rpc_object.SerializeToString(rpc_call.mutable_call_data());
      client_->CallMethod(rpc_call, nullptr);
    }
    catch (...) {
      // not yet supported
    }
  }
}

void ProductInfoInterface_Proxy::GetProductInfo(ProductInfo *out__) {
  nanorpc::RpcCall rpc_call__;
  if (object_id_ != 0) {
    rpc_call__.set_object_id(object_id_);
  } else {
    rpc_call__.set_service("arizona_api_service.ProductInfoInterface");
  }

  rpc_call__.set_method("GetProductInfo");
  nanorpc::RpcResult rpc_result__;
  client_->CallMethod(rpc_call__, &rpc_result__);
  if (out__ != nullptr)
    out__->ParseFromString(rpc_result__.result_data());
}

SoftwareUpdateInterface_Proxy::~SoftwareUpdateInterface_Proxy() {
  if (object_id_ != 0) {
    try {
      nanorpc::RpcCall rpc_call;
      rpc_call.set_service("NanoRpc.ObjectManagerService");
      rpc_call.set_method("Delete");
      nanorpc::RpcObject rpc_object;
      rpc_object.set_object_id(object_id_);
      rpc_object.SerializeToString(rpc_call.mutable_call_data());
      client_->CallMethod(rpc_call, nullptr);
    }
    catch (...) {
      // not yet supported
    }
  }
}

void SoftwareUpdateInterface_Proxy::GetAvailablePackages(SoftwarePackageList *out__) {
  nanorpc::RpcCall rpc_call__;
  if (object_id_ != 0) {
    rpc_call__.set_object_id(object_id_);
  } else {
    rpc_call__.set_service("arizona_api_service.SoftwareUpdateInterface");
  }

  rpc_call__.set_method("GetAvailablePackages");
  nanorpc::RpcResult rpc_result__;
  client_->CallMethod(rpc_call__, &rpc_result__);
  if (out__ != nullptr)
    out__->ParseFromString(rpc_result__.result_data());
}

bool SoftwareUpdateInterface_Proxy::StartPackageInstallation(const std::string &value) {
  nanorpc::RpcCall rpc_call__;
  if (object_id_ != 0) {
    rpc_call__.set_object_id(object_id_);
  } else {
    rpc_call__.set_service("arizona_api_service.SoftwareUpdateInterface");
  }

  rpc_call__.set_method("StartPackageInstallation");
  google::protobuf::StringValue in_arg__;

  in_arg__.set_value(value);

  in_arg__.SerializeToString(rpc_call__.mutable_call_data());
  nanorpc::RpcResult rpc_result__;
  client_->CallMethod(rpc_call__, &rpc_result__);
  google::protobuf::BoolValue out_pb__;
  out_pb__.ParseFromString(rpc_result__.result_data());
  return out_pb__.value();
}

bool SoftwareUpdateInterface_Proxy::DeletePackage(const std::string &value) {
  nanorpc::RpcCall rpc_call__;
  if (object_id_ != 0) {
    rpc_call__.set_object_id(object_id_);
  } else {
    rpc_call__.set_service("arizona_api_service.SoftwareUpdateInterface");
  }

  rpc_call__.set_method("DeletePackage");
  google::protobuf::StringValue in_arg__;

  in_arg__.set_value(value);

  in_arg__.SerializeToString(rpc_call__.mutable_call_data());
  nanorpc::RpcResult rpc_result__;
  client_->CallMethod(rpc_call__, &rpc_result__);
  google::protobuf::BoolValue out_pb__;
  out_pb__.ParseFromString(rpc_result__.result_data());
  return out_pb__.value();
}

void SoftwareUpdateInterface_Proxy::GetPackageStorePath(std::string *out__) {
  nanorpc::RpcCall rpc_call__;
  if (object_id_ != 0) {
    rpc_call__.set_object_id(object_id_);
  } else {
    rpc_call__.set_service("arizona_api_service.SoftwareUpdateInterface");
  }

  rpc_call__.set_method("GetPackageStorePath");
  nanorpc::RpcResult rpc_result__;
  client_->CallMethod(rpc_call__, &rpc_result__);
  if (out__ != nullptr) {
    google::protobuf::StringValue out_pb__;
    out_pb__.ParseFromString(rpc_result__.result_data());
    *out__ = out_pb__.value();
  }
}

}  // namespace arizona_api_service


