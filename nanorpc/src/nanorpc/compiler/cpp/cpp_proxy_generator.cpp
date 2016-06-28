#include "nanorpc\compiler\cpp\cpp_generator.h"

#include <string>
#include <vector>

#include "google\protobuf\io\printer.h"
#include "google\protobuf\io\zero_copy_stream_impl_lite.h"

#include "nanorpc\compiler\cpp\code_model.h"

namespace pb = ::google::protobuf;

void GenerateEventProxyDeclaration(
    pb::io::Printer &printer,
    const code_model::ServiceModel &service_model) {
  std::map<std::string, std::string> vars;
  vars["service_name"] = service_model.name();

  // TODO: Differentiate between _Proxy and _EventProxy
  //  - Event proxy has the following constructor
  //       .ctor(nanorpc::IRpcMessageSender *sender) : sender_( sender ) {}
  //  - Event proxy has empty destructor
  //  - Event proxy has one member
  //      nanorpc::IRpcMessageSender *sender_;
  /* clang-format off */
  printer.Print(vars, "class $service_name$_EventProxy : public $service_name$ {\n");
  printer.Print(vars, "public:\n");
  printer.Indent();
  printer.Print(vars, "explicit $service_name$_EventProxy(nanorpc::EventSourceInterface *event_source)\n");
  printer.Print(vars, "    : event_source_(event_source) {}\n\n");

  printer.Print(vars, "virtual ~$service_name$_EventProxy() {}\n\n");
  /* clang-format on */

  for (const auto &method_model : service_model.methods()) {
    vars["method_name"] = method_model.name();
    if (method_model.is_property()) {
      assert(false);
    } else {
      vars["method_signature"] =
          GetMethodSignature(method_model, std::string());
      printer.Print(vars, "$method_signature$ override;\n");
    }
  }

  printer.Print(vars, "\n");

  printer.Outdent();
  printer.Print(vars, "private:\n");
  printer.Indent();
  printer.Print(vars, "nanorpc::EventSourceInterface *event_source_;\n");
  printer.Outdent();
  printer.Print(vars, "};\n\n");
}

void GenerateProxyDeclaration(pb::io::Printer &printer,
                              const code_model::ServiceModel &service_model) {
  std::map<std::string, std::string> vars;
  vars["service_name"] = service_model.name();

  /* clang-format off */
  printer.Print(vars, "class $service_name$_Proxy : public $service_name$ {\n");
  printer.Print(vars, "public:\n");
  printer.Indent();
  printer.Print(vars, "explicit $service_name$_Proxy(nanorpc::ServiceProxyInterface *client, nanorpc::RpcObjectId object_id = 0)\n");
  printer.Print(vars, "    : client_(client), object_id_(object_id) {}\n\n");

  printer.Print(vars, "virtual ~$service_name$_Proxy();\n\n");
  /* clang-format on */

  for (const auto &method_model : service_model.methods()) {
    vars["method_name"] = method_model.name();
    if (method_model.is_property()) {
      if (method_model.getter() != nullptr)
        vars["getter_signature"] =
            GetPropertySignature(method_model.getter(), false);

      if (method_model.setter() != nullptr)
        vars["setter_signature"] =
            GetPropertySignature(method_model.setter(), true);

      printer.Print(vars, "$getter_signature$ override;\n");
      printer.Print(vars, "$setter_signature$ override;\n");
    } else {
      vars["method_signature"] =
          GetMethodSignature(method_model, std::string());
      printer.Print(vars, "$method_signature$ override;\n");
    }
  }

  printer.Print(vars, "\n");

  printer.Outdent();
  printer.Print(vars, "private:\n");
  printer.Indent();
  printer.Print(vars, "nanorpc::ServiceProxyInterface *client_;\n");
  printer.Print(vars, "nanorpc::RpcObjectId object_id_;\n");
  printer.Outdent();
  printer.Print(vars, "};\n\n");
}

std::string GetProxyDeclarations(
    const std::vector<code_model::ServiceModel> &models) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    pb::io::StringOutputStream output_stream(&output);
    pb::io::Printer printer(&output_stream, '$');
    std::map<std::string, std::string> vars;

    for (const auto &service_model : models) {
      if (service_model.is_event_source())
        GenerateEventProxyDeclaration(printer, service_model);
      else
        GenerateProxyDeclaration(printer, service_model);
    }
  }
  return output;
}

void GenerateProxyMethodImplementation(pb::io::Printer &printer,
                                       const std::string &full_service_name,
                                       bool is_event_source,
                                       const code_model::MethodModel &method) {
  std::map<std::string, std::string> vars;

  vars["full_service_name"] = full_service_name;
  vars["method_name"] = method.name();
  vars["return_wrapper_name"] = method.return_type().wrapper_name();

  /* clang-format off */
  printer.Print(vars, "nanorpc::RpcCall rpc_call__;\n");
  if (is_event_source) {
    printer.Print(vars, "rpc_call__.set_service(\"$full_service_name$\");\n");
  } else {
    printer.Print(vars, "if (object_id_ != 0) {\n");
    printer.Indent();
    printer.Print(vars, "rpc_call__.set_object_id(object_id_);\n");
    printer.Outdent();
    printer.Print(vars, "} else {\n");
    printer.Indent();
    printer.Print(vars, "rpc_call__.set_service(\"$full_service_name$\");\n");
    printer.Outdent();
    printer.Print(vars, "}\n\n");
  }

  printer.Print(vars, "rpc_call__.set_method(\"$method_name$\");\n");
  /* clang-format on */

  // Serialize input arguments
  if (method.is_arglist()) {
    vars["arglist_typename"] = method.arglist_typename();
    printer.Print(vars, "$arglist_typename$ args__;\n");

    for (size_t j = 0; j < method.arguments().size(); ++j) {
      const auto &arg = method.arguments()[j];
      vars["arg_name"] = arg.name();
      /* clang-format off */
      if (arg.type().is_struct())
        printer.Print(vars, "*args__.mutable_$arg_name$() = $arg_name$;\n");
      else
        printer.Print(vars, "args__.set_$arg_name$($arg_name$);\n");
      /* clang-format on */
    }

    /* clang-format off */
    printer.Print(vars, "args__.SerializeToString(rpc_call__.mutable_call_data());\n");
    printer.Print(vars, "\n");
    /* clang-format on */
  } else if (method.arguments().size() == 1) {
    const auto &arg = method.arguments()[0];

    // Declare wrapper variable for single argument calls and arglist calls.
    vars["arg_type"] = arg.type().wrapper_name();
    vars["arg_name"] = arg.name();
    if (arg.type().is_struct()) {
      printer.Print(vars, "const $arg_type$ &in_arg__ = value;\n\n");
    } else {
      printer.Print(vars, "$arg_type$ in_arg__;\n\n");
      printer.Print(vars, "in_arg__.set_$arg_name$(value);\n\n");
    }
    /* clang-format off */
    printer.Print(vars, "in_arg__.SerializeToString(rpc_call__.mutable_call_data());\n");
    /* clang-format on */
  }

  if (is_event_source) {
    printer.Print(vars, "event_source_->SendEvent(rpc_call__);\n");
  } else {
    printer.Print(vars, "nanorpc::RpcResult rpc_result__;\n");
    printer.Print(vars, "client_->CallMethod(rpc_call__, &rpc_result__);\n");
  }

  // Deserialize the result
  if (!method.return_type().is_void()) {
    if (is_event_source)
      assert(false);  // Event source methods cannot have return type.

    if (method.return_type().is_struct()) {
      /* clang-format off */
      printer.Print(vars, "if (out__ != nullptr)\n");
      printer.Indent();
      printer.Print(vars, "out__->ParseFromString(rpc_result__.result_data());\n");
      printer.Outdent();
      /* clang-format on */
    } else if (method.return_type().is_reference_type()) {
      /* clang-format off */
      printer.Print(vars, "if (out__ != nullptr) {\n");
      printer.Indent();
      printer.Print(vars, "$return_wrapper_name$ out_pb__;\n");
      printer.Print(vars, "out_pb__.ParseFromString(rpc_result__.result_data());\n");
      printer.Print(vars, "*out__ = out_pb__.value();\n");
      printer.Outdent();
      printer.Print(vars, "}\n");
      /* clang-format on */
    } else {
      /* clang-format off */
      printer.Print(vars, "$return_wrapper_name$ out_pb__;\n");
      printer.Print(vars, "out_pb__.ParseFromString(rpc_result__.result_data());\n");
      printer.Print(vars, "return out_pb__.value();\n");
      /* clang-format on */
    }
  }
}

void GenerateProxyImplementation(pb::io::Printer &printer,
                                 const code_model::ServiceModel &service) {
  for (const auto &method : service.methods()) {
    std::map<std::string, std::string> vars;

    vars["method_name"] = method.name();
    vars["method_signature"] =
        service.is_event_source()
            ? GetMethodSignature(method, service.name() + "_EventProxy")
            : GetMethodSignature(method, service.name() + "_Proxy");
    vars["return_type"] = "void";

    printer.Print(vars, "$method_signature$ {\n");
    printer.Indent();
    GenerateProxyMethodImplementation(printer, service.full_name(),
                                      service.is_event_source(), method);
    printer.Outdent();
    printer.Print(vars, "}\n\n");
  }
}

std::string GetProxyDefinitions(
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

      if (!service.is_event_source()) {
        // clang-format off
        printer.Print(vars, "$service_name$_Proxy::~$service_name$_Proxy() {\n");
        printer.Indent();
        printer.Print(vars, "if (object_id_ != 0) {\n");
        printer.Indent();
        printer.Print(vars, "try {\n");
        printer.Indent();
        printer.Print(vars, "nanorpc::RpcCall rpc_call;\n");
        printer.Print(vars, "rpc_call.set_service(\"NanoRpc.ObjectManagerService\");\n");
        printer.Print(vars, "rpc_call.set_method(\"Delete\");\n");
        printer.Print(vars, "nanorpc::RpcObject rpc_object;\n");
        printer.Print(vars, "rpc_object.set_object_id(object_id_);\n");
        printer.Print(vars, "rpc_object.SerializeToString(rpc_call.mutable_call_data());\n");
        printer.Print(vars, "client_->CallMethod(rpc_call, nullptr);\n");
        printer.Outdent();
        printer.Print(vars, "}\n");
        printer.Print(vars, "catch (...) {\n");
        printer.Indent();
        printer.Print(vars, "// not yet supported\n");
        printer.Outdent();
        printer.Print(vars, "}\n");
        printer.Outdent();
        printer.Print(vars, "}\n");
        printer.Outdent();
        printer.Print(vars, "}\n\n");
        // clang-format on
      }

      GenerateProxyImplementation(printer, service);
    }
  }
  return output;
}
