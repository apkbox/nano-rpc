#if !defined(NANORPC_NANORPC_CPP_PLUGIN_H__)
#define NANORPC_NANORPC_CPP_PLUGIN_H__

#include <memory>
#include <string>
#include <vector>

#include "google\protobuf\compiler\plugin.pb.h"

namespace code_model {

class TypeModel {
public:
  TypeModel(const google::protobuf::Descriptor &type) {
    ParseFromDescriptor(&type);
  }
  TypeModel() : is_reference_type_(false), is_struct_(false), is_void_(true) {}

  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }

  const std::string &wrapper_name() const { return wrapper_name_; }
  void set_wrapper_name(const std::string &name) { wrapper_name_ = name; }

  bool is_reference_type() const { return is_reference_type_; }
  void set_reference_type(bool is_reference_type) {
    is_reference_type_ = is_reference_type;
  }

  bool is_struct() const { return is_struct_; }

  bool is_void() const { return is_void_; }
  void set_void(bool is_void) { is_void_ = is_void; }

  bool ParseFromDescriptor(const google::protobuf::Descriptor *type);
  bool ParseFromFieldDescriptor(const google::protobuf::FieldDescriptor *field);

private:
  std::string name_;
  std::string wrapper_name_;
  bool is_reference_type_;
  bool is_struct_;
  bool is_void_;
};

class ArgumentModel {
public:
  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }

  const TypeModel &type() const { return type_; }
  void set_type(const TypeModel &type) { type_ = type; }
  TypeModel *mutable_type() { return &type_; }

private:
  std::string name_;
  TypeModel type_;
};

class MethodModel {
public:
  MethodModel() : is_property_(false), is_arglist_(false) {}
  MethodModel(const MethodModel &other)
      : name_(other.name_),
        is_property_(other.is_property_),
        is_arglist_(other.is_arglist_),
        arglist_typename_(other.arglist_typename_),
        arguments_(other.arguments_),
        return_type_(other.return_type_) {
    if (other.getter_ != nullptr)
      getter_.reset(new MethodModel(*other.getter_.get()));
    if (other.setter_ != nullptr)
      setter_.reset(new MethodModel(*other.setter_.get()));
  }

  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }

  bool is_property() const { return is_property_; }
  void set_property(bool is_property) { is_property_ = is_property; }

  bool is_arglist() const { return is_arglist_; }
  void set_arglist(bool value) { is_arglist_ = value; }

  const std::string &arglist_typename() const { return arglist_typename_; }
  void set_arglist_typename(const std::string &value) { arglist_typename_ = value; }

  const std::vector<ArgumentModel> &arguments() const { return arguments_; }
  std::vector<ArgumentModel> *mutable_arguments() { return &arguments_; }

  const TypeModel &return_type() const { return return_type_; }
  void set_return_type(const TypeModel &type_model) {
    return_type_ = type_model;
  }
  TypeModel *mutable_return_type() { return &return_type_; }

  const MethodModel *getter() const { return getter_.get(); }
  void set_getter(const MethodModel &method) {
    getter_.reset(new MethodModel(method));
  }

  const MethodModel *setter() const { return setter_.get(); }
  void set_setter(const MethodModel &method) {
    setter_.reset(new MethodModel(method));
  }

private:
  std::string name_;
  bool is_property_;
  bool is_arglist_;
  std::string arglist_typename_;
  std::vector<ArgumentModel> arguments_;
  TypeModel return_type_;
  std::unique_ptr<MethodModel> getter_;
  std::unique_ptr<MethodModel> setter_;
};

class ServiceModel {
public:
  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }

  const std::string &full_name() const { return full_name_; }
  void set_full_name(const std::string &name) { full_name_ = name; }

  const std::vector<MethodModel> &methods() const { return methods_; }
  std::vector<MethodModel> *mutable_methods() { return &methods_; }

private:
  std::string name_;
  std::string full_name_;
  std::vector<MethodModel> methods_;
};

bool CreateCodeModel(const google::protobuf::FileDescriptor *file,
                     std::vector<ServiceModel> *service_models);
}  // namespace code_model

#endif  // NANORPC_NANORPC_CPP_PLUGIN_H__
