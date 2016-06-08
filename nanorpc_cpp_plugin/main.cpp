#include <string>
#include <vector>

#include "google\protobuf\io\coded_stream.h"
#include "google\protobuf\io\zero_copy_stream.h"
#include "google\protobuf\io\zero_copy_stream_impl_lite.h"
#include "google\protobuf\io\printer.h"
#include "google\protobuf\compiler\plugin.h"
#include "google\protobuf\compiler\plugin.pb.h"
#include "google\protobuf\compiler\code_generator.h"
#include "google\protobuf\compiler\cpp\cpp_generator.h"

#include "generator_utils.h"
#include "code_model.h"

namespace pb = google::protobuf;
namespace pbc = google::protobuf::compiler;

std::string GetHeaderPrologue(const pb::FileDescriptor *file);
std::string GetInterfaceDefinitions(const pb::FileDescriptor *file, const std::vector<code_model::ServiceModel> &models);
std::string GetStubDeclarations(const pb::FileDescriptor *file);
std::string GetHeaderEpilogue(const pb::FileDescriptor *file);

std::string GetSourcePrologue(const pb::FileDescriptor *file);
std::string GetStubDefinitions(const pb::FileDescriptor *file, const std::vector<code_model::ServiceModel> &models);
std::string GetSourceEpilogue(const pb::FileDescriptor *file);

class NanoRpcCppGenerator : public pbc::CodeGenerator {
public:
  bool Generate(const pb::FileDescriptor *file,
                const std::string &parameter,
                pbc::GeneratorContext *context,
                std::string *error) const override;
};

bool NanoRpcCppGenerator::Generate(const pb::FileDescriptor *file,
                                   const std::string &parameter,
                                   pbc::GeneratorContext *context,
                                   std::string *error) const {
  std::vector<code_model::ServiceModel> service_models;
  if (!code_model::CreateCodeModel(file, &service_models))
    return false;

  std::string file_name = StripProto(file->name());

  std::string header_code = GetHeaderPrologue(file) +
                            GetInterfaceDefinitions(file, service_models) +
                            GetStubDeclarations(file) + GetHeaderEpilogue(file);
  std::unique_ptr<pb::io::ZeroCopyOutputStream> header_output(
      context->Open(file_name + ".nanorpc.pb.h"));
  pb::io::CodedOutputStream header_coded_out(header_output.get());
  header_coded_out.WriteRaw(header_code.data(), header_code.size());

  std::string source_code = GetSourcePrologue(file) + GetStubDefinitions(file, service_models) +
                            GetSourceEpilogue(file);
  std::unique_ptr<pb::io::ZeroCopyOutputStream> source_output(
      context->Open(file_name + ".nanorpc.pb.cc"));
  pb::io::CodedOutputStream source_coded_out(source_output.get());
  source_coded_out.WriteRaw(source_code.data(), source_code.size());

  return true;
}

int main(int argc, char **argv) {
  NanoRpcCppGenerator generator;
  return pbc::PluginMain(argc, argv, &generator);
}
