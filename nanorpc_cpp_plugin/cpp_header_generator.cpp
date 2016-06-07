#include <string>

#include "google\protobuf\compiler\code_generator.h"
#include "google\protobuf\compiler\cpp\cpp_generator.h"
#include "google\protobuf\compiler\plugin.h"
#include "google\protobuf\compiler\plugin.pb.h"
#include "google\protobuf\io\coded_stream.h"
#include "google\protobuf\io\printer.h"
#include "google\protobuf\io\zero_copy_stream.h"
#include "google\protobuf\io\zero_copy_stream_impl_lite.h"
#include "google\protobuf\wrappers.pb.h"

#include "rpc_proto/rpc_types.pb.h"

#include "generator_utils.h"

namespace pb = google::protobuf;
namespace pbc = google::protobuf::compiler;

// Returns true if type can be passed by value.
// TODO: Rewrite this to return comprehensive structure with
// the following information:
//    - C++ type name
//    - Possible type names for other lanuages
//    - Whether the type is a value type
//    - Whether the type is void
//    - If type is a message, whether the message is an argument list.
bool GetCppType(const pb::Descriptor *type, std::string *cpp_type_name) {
  if (type->full_name() == nanorpc2::RpcVoid::descriptor()->full_name()) {
    *cpp_type_name = "void";
    return true;
  } else if (type->full_name() == pb::BoolValue::descriptor()->full_name()) {
    *cpp_type_name = "bool";
    return true;
  } else if (type->full_name() == pb::Int32Value::descriptor()->full_name()) {
    *cpp_type_name = "int32_t";
    return true;
  } else if (type->full_name() == pb::Int64Value::descriptor()->full_name()) {
    *cpp_type_name = "int64_t";
    return true;
  } else if (type->full_name() == pb::UInt32Value::descriptor()->full_name()) {
    *cpp_type_name = "uint32_t";
    return true;
  } else if (type->full_name() == pb::UInt64Value::descriptor()->full_name()) {
    *cpp_type_name = "uint64_t";
    return true;
  } else if (type->full_name() == pb::StringValue::descriptor()->full_name()) {
    *cpp_type_name = "std::string";
    return false;
  }

  if (type->options().HasExtension(nanorpc2::enum_wrapper) &&
      type->options().GetExtension(nanorpc2::enum_wrapper)) {
    // We expect exactly one field in the enum wrapper and this should be
    // an enumeration.
    assert(type->field_count() == 1);
    assert(type->field(0)->type() == pb::FieldDescriptor::TYPE_ENUM);
    if (type->field_count() != 1 ||
        type->field(0)->type() == pb::FieldDescriptor::TYPE_ENUM) {
      /* TODO: Fail compilation */
    }

    *cpp_type_name = type->field(0)->enum_type()->name();
    return true;
  }

  *cpp_type_name = type->name();

  return false;
}

std::string GetPropertySignature(const pb::MethodDescriptor *method,
                                 bool setter) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    pb::io::StringOutputStream output_stream(&output);
    pb::io::Printer printer(&output_stream, '$');
    std::map<std::string, std::string> vars;

    const pb::Descriptor *type =
        setter ? method->input_type() : method->output_type();

    std::string cpp_type_name;
    bool is_value_type = GetCppType(type, &cpp_type_name);

    vars["method_name"] = method->name();
    vars["type_name"] = cpp_type_name;

    if (is_value_type) {
      if (setter)
        printer.Print(vars, "void set_$method_name$($type_name$ value)");
      else
        printer.Print(vars, "$type_name$ get_$method_name$() const");
    } else {
      if (setter)
        printer.Print(vars, "void set_$method_name$(const $type_name$ &value)");
      else
        printer.Print(vars, "void get_$method_name$($type_name$ *value) const");
    }
  }

  return output;
}

std::string MessageToArgumentList(const pb::Descriptor *message) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    pb::io::StringOutputStream output_stream(&output);
    pb::io::Printer printer(&output_stream, '$');

    for (int i = 0; i < message->field_count(); ++i) {
      std::map<std::string, std::string> vars;

      const pb::FieldDescriptor *field = message->field(i);
      if (field->type() == pb::FieldDescriptor::TYPE_MESSAGE) {
        std::string cpp_type_name;
        bool is_value_type = GetCppType(field->message_type(), &cpp_type_name);
        vars["arg_type"] = cpp_type_name;
        vars["const"] = "";
        vars["reference"] = "";

        if (!is_value_type) {
          vars["const"] = "const ";
          vars["reference"] = "&";
        }
      } else {
        vars["arg_type"] = field->cpp_type_name();
      }

      vars["arg_name"] = field->name();
      printer.Print(vars, "$const$$arg_type$ $reference$$arg_name$");
      if ((i + 1) < message->field_count())
        printer.Print(", ");
    }
  }

  return output;
}

std::string GetMethodSignature(const pb::MethodDescriptor *method) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    pb::io::StringOutputStream output_stream(&output);
    pb::io::Printer printer(&output_stream, '$');
    std::map<std::string, std::string> vars;

    const pb::Descriptor *input_type = method->input_type();
    const pb::Descriptor *output_type = method->output_type();

    std::string input_type_name;
    std::string output_type_name;
    bool input_is_value_type = GetCppType(input_type, &input_type_name);
    bool expanded_args = false;
    bool output_is_value_type = GetCppType(output_type, &output_type_name);

    if (input_type->options().HasExtension(nanorpc2::expand_as_arguments) &&
        input_type->options().GetExtension(nanorpc2::expand_as_arguments)) {
      vars["input_args"] = MessageToArgumentList(input_type);
      expanded_args = true;
    }

    vars["method_name"] = method->name();
    vars["input_type_name"] = input_type_name;
    vars["output_type_name"] = output_type_name;

    bool has_arguments = false;

    if (output_is_value_type)
      printer.Print(vars, "$output_type_name$ $method_name$(");
    else
      printer.Print(vars, "void $method_name$(");

    if (input_is_value_type) {
      const std::string &void_type_name = nanorpc2::RpcVoid::descriptor()->full_name();

      if (input_type->full_name() != void_type_name) {
        printer.Print(vars, "$input_type_name$ value");
        has_arguments = true;
      }
    } else if (expanded_args) {
      printer.Print(vars, "$input_args$");
      has_arguments = true;
    } else {
      printer.Print(vars, "const $input_type_name$ &value");
      has_arguments = true;
    }

    if (!output_is_value_type) {
      if (has_arguments)
        printer.Print(", ");
      printer.Print(vars, "$output_type_name$ *output_value)");
    } else {
      printer.Print(")");
    }
  }

  return output;
}

std::string GetHeaderPrologue(const pb::FileDescriptor *file) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    pb::io::StringOutputStream output_stream(&output);
    pb::io::Printer printer(&output_stream, '$');
    std::map<std::string, std::string> vars;

    vars["filename"] = file->name();
    vars["filename_identifier"] = FilenameIdentifier(file->name());
    vars["filename_base"] = StripProto(file->name());

    /* clang-format off */
    printer.Print(vars, "// Generated by the nanorpc protobuf plugin.\n");
    printer.Print(vars, "// If you make any local change, they will be lost.\n");
    printer.Print(vars, "// source: $filename$\n");
    printer.Print(vars, "#ifndef NANORPC_$filename_identifier$__INCLUDED\n");
    printer.Print(vars, "#define NANORPC_$filename_identifier$__INCLUDED\n");
    printer.Print(vars, "\n");
    printer.Print(vars, "#include \"$filename_base$.pb.h\"\n");
    printer.Print(vars, "#include \"rpc_stub.hpp\"\n");
    printer.Print(vars, "#include \"rpc_object_manager.hpp\"\n");
    printer.Print(vars, "\n");
    /* clang-format on */

    if (!file->package().empty()) {
      std::vector<std::string> parts = tokenize(file->package(), ".");

      for (auto part = parts.rbegin(); part != parts.rend(); part++) {
        vars["part"] = *part;
        printer.Print(vars, "namespace $part$ {\n");
      }

      printer.Print(vars, "\n");
    }
  }
  return output;
}

std::string GetInterfaceDefinitions(const pb::FileDescriptor *file) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    pb::io::StringOutputStream output_stream(&output);
    pb::io::Printer printer(&output_stream, '$');
    std::map<std::string, std::string> vars;

    for (int i = 0; i < file->service_count(); ++i) {
      auto service = file->service(i);
      vars["service_name"] = service->name();

      /* clang-format off */
      printer.Print(vars, "class $service_name$ {\n");
      printer.Print(vars, "public:\n");
      printer.Print(vars, "  virtual ~$service_name$() {}\n\n");

      for (int j = 0; j < service->method_count(); ++j) {
        auto method = service->method(j);
        vars["method_name"] = method->name();
        if (method->options().HasExtension(nanorpc2::is_property) && 
            method->options().GetExtension(nanorpc2::is_property)) {
          bool no_setter = false;
          bool no_getter = false;
          const std::string &void_type_name = nanorpc2::RpcVoid::descriptor()->full_name();

          if (method->input_type()->full_name() == void_type_name)
            no_setter = true;

          if (method->output_type()->full_name() == void_type_name)
            no_getter = true;

          assert(!no_getter && !no_setter);
          if (no_getter && no_setter) {
            printer.Print(vars, "#error \"Invalid '$method_name$' property definition\"\n");
            /* TODO: Fail the compilation. */
          }

          if (!no_getter)
            vars["getter_signature"] = GetPropertySignature(method, false);
          if (!no_setter)
            vars["setter_signature"] = GetPropertySignature(method, true);

          printer.Print(vars, "  virtual $getter_signature$ = 0;\n");
          printer.Print(vars, "  virtual $setter_signature$ = 0;\n");
        }
        else {
          vars["method_signature"] = GetMethodSignature(method);
          printer.Print(vars, "  virtual $method_signature$ = 0;\n");
        }
      }

      printer.Print(vars, "};\n\n");
      /* clang-format on */
    }
  }
  return output;
}

std::string GetStubDefinitions(const pb::FileDescriptor *file) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    pb::io::StringOutputStream output_stream(&output);
    pb::io::Printer printer(&output_stream, '$');
    std::map<std::string, std::string> vars;

    for (int i = 0; i < file->service_count(); ++i) {
      auto service = file->service(i);
      vars["service_name"] = service->name();

      // clang-format off
      printer.Print(vars, "class $service_name$_Stub : public nanorpc2::IRpcStub {\n");
      printer.Print(vars, "public:\n");
      printer.Print(vars, "  explicit $service_name$_Stub(nanorpc2::IRpcObjectManager* object_manager, $service_name$* impl)\n");
      printer.Print(vars, "      : object_manager_(object_manager), impl_(impl) {}\n\n");
      printer.Print(vars, "  const char *GetInterfaceName() const;\n");
      printer.Print(vars, "  void CallMethod(const nanorpc2::RpcCall &rpc_call, nanorpc2::RpcResult *rpc_result);\n\n");
      printer.Print(vars, "private:\n");
      printer.Print(vars, "  nanorpc2::IRpcObjectManager* object_manager_;\n");
      printer.Print(vars, "  $service_name$* impl_;\n");
      printer.Print(vars, "};\n\n");
      // clang-format on
    }
  }
  return output;
}

std::string GetHeaderEpilogue(const pb::FileDescriptor *file) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    pb::io::StringOutputStream output_stream(&output);
    pb::io::Printer printer(&output_stream, '$');
    std::map<std::string, std::string> vars;

    vars["filename"] = file->name();
    vars["filename_identifier"] = FilenameIdentifier(file->name());

    if (!file->package().empty()) {
      std::vector<std::string> parts = tokenize(file->package(), ".");

      for (auto part = parts.rbegin(); part != parts.rend(); part++) {
        vars["part"] = *part;
        printer.Print(vars, "}  // namespace $part$\n");
      }
      printer.Print(vars, "\n");
    }

    printer.Print(vars, "\n");
    printer.Print(vars, "#endif  // NANORPC_$filename_identifier$__INCLUDED\n");
  }
  return output;
}
