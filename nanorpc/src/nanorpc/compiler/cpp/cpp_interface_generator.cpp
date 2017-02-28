#include "nanorpc\compiler\cpp\cpp_generator.h"

#include <string>
#include <vector>

#include "google\protobuf\io\printer.h"
#include "google\protobuf\io\zero_copy_stream_impl_lite.h"

#include "nanorpc\compiler\cpp\code_model.h"

namespace pb = ::google::protobuf;

std::string GetInterfaceDefinitions(
    const std::vector<code_model::ServiceModel> &models) {
  std::string output;
  {
    // Scope the output stream so it closes and finalizes output to the string.
    pb::io::StringOutputStream output_stream(&output);
    pb::io::Printer printer(&output_stream, '$');
    std::map<std::string, std::string> vars;

    for (const auto &service_model : models) {
      vars["service_name"] = service_model.name();

      printer.Print(vars, "class $service_name$ {\n");
      printer.Print(vars, "public:\n");
      printer.Indent();
      printer.Print(vars, "virtual ~$service_name$() {}\n\n");

      for (const auto &method_model : service_model.methods()) {
        vars["method_name"] = method_model.name();
        if (method_model.is_property()) {
          if (method_model.getter() != nullptr)
            vars["getter_signature"] =
                GetPropertySignature(method_model.getter(), false, true);

          if (method_model.setter() != nullptr)
            vars["setter_signature"] =
                GetPropertySignature(method_model.setter(), true, true);

          printer.Print(vars, "virtual $getter_signature$ = 0;\n");
          printer.Print(vars, "virtual $setter_signature$ = 0;\n");
        } else {
          vars["method_signature"] =
              GetMethodSignature(method_model, std::string(), true);
          printer.Print(vars, "virtual $method_signature$ = 0;\n");
        }
      }

      printer.Outdent();
      printer.Print(vars, "};\n\n");
    }
  }
  return output;
}
