#include "nanorpc\compiler\cpp\cpp_generator.h"

#include <string>
#include <vector>

#include "google\protobuf\io\printer.h"
#include "google\protobuf\io\zero_copy_stream_impl_lite.h"

#include "nanorpc\compiler\cpp\code_model.h"

namespace pb = ::google::protobuf;

std::string GetStubDeclarations(const pb::FileDescriptor *file) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    pb::io::StringOutputStream output_stream(&output);
    pb::io::Printer printer(&output_stream, '$');
    std::map<std::string, std::string> vars;

    for (int i = 0; i < file->service_count(); ++i) {
      const auto service = file->service(i);
      vars["service_name"] = service->name();

      /* clang-format off */
      printer.Print(vars, "class $service_name$_Stub : public nanorpc::ServiceInterface {\n");
      printer.Print(vars, "public:\n");
      printer.Indent();
      printer.Print(vars, "explicit $service_name$_Stub($service_name$* impl, nanorpc::ObjectManagerInterface* object_manager)\n");
      printer.Print(vars, "    : impl_(impl), object_manager_(object_manager) {}\n\n");
      printer.Print(vars, "const std::string &GetInterfaceName() const override;\n");
      printer.Print(vars, "bool CallMethod(const nanorpc::RpcCall &rpc_call, nanorpc::RpcResult *rpc_result) override;\n\n");
      printer.Outdent();
      printer.Print(vars, "private:\n");
      printer.Indent();
      printer.Print(vars, "static const std::string kServiceName;\n\n");
      printer.Print(vars, "$service_name$* impl_;\n");
      printer.Print(vars, "nanorpc::ObjectManagerInterface* object_manager_;\n");
      printer.Outdent();
      printer.Print(vars, "};\n\n");
      /* clang-format on */
    }
  }
  return output;
}

void GenerateStubMethodCallImplementation(
    pb::io::Printer &printer,
    const code_model::MethodModel &method) {
  std::map<std::string, std::string> vars;

  vars["method_name"] = method.name();
  vars["return_type"] = method.return_type().name();
  vars["return_wrapper_name"] = method.return_type().wrapper_name();

  // Deserialize input arguments
  if (method.is_arglist()) {
    vars["arglist_typename"] = method.arglist_typename();
    printer.Print(vars, "$arglist_typename$ args__;\n");
    // BUG: This is incorrect if argument is a value type.
    // In order to parse it out one need to declare a PB wrapper first,
    // parse and then extract the value out of the wrapper.
    printer.Print(vars, "args__.ParseFromString(rpc_call.call_data());\n\n");

    // We can avoid declaring argument variable for value
    // types and instead pass them directly via message accessor, but
    // declaring them makes it easier to debug and they probably will
    // be optimized away anyways.
    for (size_t j = 0; j < method.arguments().size(); ++j) {
      const auto &arg = method.arguments()[j];
      vars["arg_name"] = arg.name();
      vars["arg_type"] = arg.type().name();
      vars["const"] = arg.type().is_reference_type() ? "const " : "";
      vars["ref"] = arg.type().is_reference_type() ? "&" : "";
      /* clang-format off */
      printer.Print(vars, "$const$$arg_type$ $ref$$arg_name$ = args__.$arg_name$();\n");
      /* clang-format on */
    }
  } else if (method.arguments().size() == 1) {
    const auto &arg = method.arguments()[0];

    // Declare wrapper variable for single argument calls and arglist calls.
    vars["arg_type"] = arg.type().wrapper_name();
    vars["arg_name"] = arg.name();
    printer.Print(vars, "$arg_type$ in_arg__;\n\n");

    vars["arg_type"] = arg.type().name();
    vars["const"] = arg.type().is_reference_type() ? "const " : "";
    vars["ref"] = arg.type().is_reference_type() ? "&" : "";
    printer.Print(vars, "in_arg__.ParseFromString(rpc_call.call_data());\n");
    if (arg.type().is_struct())
      printer.Print(vars, "$const$$arg_type$ &$arg_name$ = in_arg__;\n");
    else
      printer.Print(vars, "$const$$arg_type$ $ref$value = in_arg__.value();\n");
  }

  if (method.arguments().size() > 0)
    printer.Print(vars, "\n");

  // Define return type variable
  if (!method.return_type().is_void()) {
    printer.Print(vars, "$return_type$ out__;\n");
    if (!method.return_type().is_struct())
      printer.Print(vars, "$return_wrapper_name$ out_pb__;\n");
  }

  // Return for value types
  if (!method.return_type().is_void() &&
      !method.return_type().is_reference_type())
    printer.Print(vars, "out__ = ");

  // Call interface method
  printer.Print(vars, "impl_->$method_name$(");

  // Specify arguments
  for (size_t j = 0; j < method.arguments().size(); ++j) {
    const auto &arg = method.arguments()[j];
    vars["arg_name"] = arg.name();
    vars["arg_type"] = arg.type().name();
    printer.Print(vars, "$arg_name$");

    if ((j + 1) < method.arguments().size())
      printer.Print(vars, ", ");
  }

  // Specify return by pointer argument (for reference types)
  if (!method.return_type().is_void() &&
      method.return_type().is_reference_type()) {
    if (method.arguments().size() > 0)
      printer.Print(vars, ", ");
    printer.Print(vars, "&out__");
  }

  printer.Print(vars, ");\n");

  // Define result wrapper variable, wrap and serialize the result
  if (!method.return_type().is_void()) {
    printer.Print(vars, "\n");
    if (method.return_type().is_struct()) {
      /* clang-format off */
      printer.Print(vars, "out__.SerializeToString(rpc_result->mutable_result_data());\n");
      /* clang-format on */
    } else {
      /* clang-format off */
      printer.Print(vars, "out_pb__.set_value(out__);\n");
      printer.Print(vars, "out_pb__.SerializeToString(rpc_result->mutable_result_data());\n");
      /* clang-format on */
    }
  }

  if (method.is_async())
    printer.Print(vars, "return false;\n");
  else
    printer.Print(vars, "return true;\n");
}

void GenerateStubImplementation(pb::io::Printer &printer,
                                const code_model::ServiceModel &service) {
  std::map<std::string, std::string> vars;

  printer.Indent();

  for (size_t i = 0; i < service.methods().size(); ++i) {
    const auto &method = service.methods()[i];
    vars["method_name"] = method.name();

    printer.Print(vars, "if (rpc_call.method() == \"$method_name$\") {\n");
    printer.Indent();
    GenerateStubMethodCallImplementation(printer, method);
    printer.Outdent();
    printer.Print(vars, "}");

    if ((i + 1) < service.methods().size())
      printer.Print(vars, " else ");
  }

  printer.Print(vars, "\n\n");

  // TODO: Generate unknown method error into result
  // TODO: Generate exception handler
  printer.Print(
      vars,
      "// TODO: Here should be unknown method error stored into rpc_result.\n");
  printer.Print(
      vars,
      "// TODO: Also an exception (code above must be guarded) result.\n\n");

  // Indicate the method is synchronous (if we reach here - no method error or
  // exception).
  printer.Print(vars, "return true;\n");
  printer.Outdent();
}

std::string GetStubDefinitions(
    const pb::FileDescriptor *file,
    const std::vector<code_model::ServiceModel> &models) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    pb::io::StringOutputStream output_stream(&output);
    pb::io::Printer printer(&output_stream, '$');
    std::map<std::string, std::string> vars;

    for (const auto &service : models) {
      vars["service_name"] = service.name();
      vars["service_full_name"] = service.full_name();

      // clang-format off
      printer.Print(vars, "const std::string $service_name$_Stub::kServiceName(\"$service_full_name$\");\n\n");

      printer.Print(vars, "const std::string &$service_name$_Stub::GetInterfaceName() const {\n");
      printer.Indent();
      printer.Print(vars, "return kServiceName;\n");
      printer.Outdent();
      printer.Print(vars, "}\n\n");

      printer.Print(vars, "bool $service_name$_Stub::CallMethod(const nanorpc::RpcCall &rpc_call, nanorpc::RpcResult *rpc_result) {\n");
      GenerateStubImplementation(printer, service);
      printer.Print(vars, "}\n\n");
      // clang-format on
    }
  }
  return output;
}
