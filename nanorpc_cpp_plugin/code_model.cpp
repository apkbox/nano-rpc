#include "code_model.h"

#include <memory>
#include <string>
#include <vector>

#include "google\protobuf\compiler\plugin.pb.h"
#include "google\protobuf\wrappers.pb.h"

#include "rpc_proto/rpc_types.pb.h"

#include "generator_utils.h"

namespace pb = google::protobuf;
namespace pbc = google::protobuf::compiler;

namespace code_model {

bool CreateTypeModel(const pb::Descriptor *type, TypeModel *type_model) {
  if (type->full_name() == nanorpc2::RpcVoid::descriptor()->full_name()) {
    type_model->set_name("void");
    type_model->set_void(true);
  } else if (type->full_name() == pb::BoolValue::descriptor()->full_name()) {
    type_model->set_name("bool");
  } else if (type->full_name() == pb::Int32Value::descriptor()->full_name()) {
    type_model->set_name("int32_t");
  } else if (type->full_name() == pb::Int64Value::descriptor()->full_name()) {
    type_model->set_name("int64_t");
  } else if (type->full_name() == pb::UInt32Value::descriptor()->full_name()) {
    type_model->set_name("uint32_t");
  } else if (type->full_name() == pb::UInt64Value::descriptor()->full_name()) {
    type_model->set_name("uint64_t");
  } else if (type->full_name() == pb::StringValue::descriptor()->full_name()) {
    type_model->set_name("std::string");
    type_model->set_reference_type(true);
  } else if (type->options().HasExtension(nanorpc2::enum_wrapper) &&
             type->options().GetExtension(nanorpc2::enum_wrapper)) {
    // We expect exactly one field in the enum wrapper and this should be
    // an enumeration.
    assert(type->field_count() == 1);
    assert(type->field(0)->type() == pb::FieldDescriptor::TYPE_ENUM);
    if (type->field_count() != 1 ||
        type->field(0)->type() == pb::FieldDescriptor::TYPE_ENUM) {
      return false;
    }

    type_model->set_name(type->field(0)->enum_type()->name());
  } else {
    type_model->set_name(type->name());
    type_model->set_reference_type(true);
  }

  return true;
}

bool MessageToArgumentList(const pb::Descriptor *message,
                           std::vector<ArgumentModel> *arguments) {
  for (int i = 0; i < message->field_count(); ++i) {
    ArgumentModel argument_model;
    const pb::FieldDescriptor *field = message->field(i);
    TypeModel type_model;
    if (!CreateTypeModel(field->message_type(), &type_model))
      return false;

    argument_model.set_name(field->name());
    argument_model.set_type(type_model);
    arguments->push_back(argument_model);
  }

  return true;
}

bool CreateMethodModel(const pb::MethodDescriptor *method,
                       MethodModel *method_model) {
  const pb::Descriptor *input_type = method->input_type();
  const pb::Descriptor *output_type = method->output_type();

  method_model->set_name(method->name());

  TypeModel input_type_model;
  if (!CreateTypeModel(input_type, &input_type_model))
    return false;

  if (input_type->options().HasExtension(nanorpc2::expand_as_arguments) &&
      input_type->options().GetExtension(nanorpc2::expand_as_arguments)) {
    if (!MessageToArgumentList(input_type, method_model->mutable_arguments()))
      return false;
  } else {
    ArgumentModel argument;
    argument.set_name("value");
    argument.set_type(input_type_model);
    method_model->mutable_arguments()->push_back(argument);
  }

  if (!CreateTypeModel(output_type, method_model->mutable_return_type()))
    return false;

  return true;
}

bool CreatePropertyModel(const pb::MethodDescriptor *method,
                         bool setter,
                         MethodModel *method_model) {
  const pb::Descriptor *type =
      setter ? method->input_type() : method->output_type();

  method_model->set_name(method->name());

  if (setter) {
    ArgumentModel argument;
    argument.set_name("value");
    if (!CreateTypeModel(type, argument.mutable_type()))
      return false;
    method_model->mutable_arguments()->push_back(argument);
  } else {
    if (!CreateTypeModel(type, method_model->mutable_return_type()))
      return false;
  }

  return true;
}

bool CreateCodeModel(const pb::FileDescriptor *file,
                     std::vector<ServiceModel> *service_models) {
  for (int i = 0; i < file->service_count(); ++i) {
    auto service = file->service(i);
    ServiceModel service_model;
    service_model.set_name(service->name());

    for (int j = 0; j < service->method_count(); ++j) {
      auto method = service->method(j);
      MethodModel method_model;
      method_model.set_name(method->name());

      if (method->options().HasExtension(nanorpc2::is_property) &&
          method->options().GetExtension(nanorpc2::is_property)) {
        bool no_setter = false;
        bool no_getter = false;
        const std::string &void_type_name =
            nanorpc2::RpcVoid::descriptor()->full_name();

        if (method->input_type()->full_name() == void_type_name)
          no_setter = true;

        if (method->output_type()->full_name() == void_type_name)
          no_getter = true;

        assert(!no_getter && !no_setter);
        if (no_getter && no_setter)
          return false;

        method_model.set_property(true);

        if (!no_getter)
          CreatePropertyModel(method, false, method_model.mutable_getter());

        if (!no_setter)
          CreatePropertyModel(method, true, method_model.mutable_setter());
      } else {
        if (!CreateMethodModel(method, &method_model))
          return false;
      }

      service_model.mutable_methods()->push_back(method_model);
    }

    service_models->push_back(service_model);
  }

  return true;
}

}  // namespace
