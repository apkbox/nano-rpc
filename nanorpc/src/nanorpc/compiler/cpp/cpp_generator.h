#if !defined(NANORPC_COMPILER_CPP_CPP_GENERATOR_H__)
#define NANORPC_COMPILER_CPP_CPP_GENERATOR_H__

#include <string>
#include <vector>

namespace code_model {
class MethodModel;
class ServiceModel;
}  // namespace code_model

namespace google {
namespace protobuf {
class FileDescriptor;
}  // namespace protobuf
}  // namespace google

// cpp_generator.cpp
std::string GetPropertySignature(const code_model::MethodModel *method,
                                 bool setter);
std::string GetMethodSignature(const code_model::MethodModel &method,
                               const std::string &service_name);
std::string GetHeaderPrologue(const ::google::protobuf::FileDescriptor *file);
std::string GetHeaderEpilogue(const ::google::protobuf::FileDescriptor *file);
std::string GetSourcePrologue(const ::google::protobuf::FileDescriptor *file);
std::string GetSourceEpilogue(const ::google::protobuf::FileDescriptor *file);

// cpp_interface_generator.cpp
std::string GetInterfaceDefinitions(
    const std::vector<code_model::ServiceModel> &models);

// cpp_proxy_generator.cpp
std::string GetProxyDeclarations(
    const std::vector<code_model::ServiceModel> &models);
std::string GetProxyDefinitions(
    const std::vector<code_model::ServiceModel> &models);

// cpp_stub_generator.cpp
std::string GetStubDeclarations(const ::google::protobuf::FileDescriptor *file);
std::string GetStubDefinitions(
    const ::google::protobuf::FileDescriptor *file,
    const std::vector<code_model::ServiceModel> &models);

#endif  // NANORPC_COMPILER_CPP_CPP_GENERATOR_H__
