@echo off
rem third_party\protobuf\bin\protoc rpc_proto\rpc_types.proto --cpp_out=NanoRpc\src
third_party\protobuf\bin\protoc app_test.proto ^
--cpp_out=NanoRpc_TestServer ^
--plugin=protoc-gen-nanorpc_cpp=x64\Debug\nanorpc_cpp_plugin.exe --nanorpc_cpp_out=NanoRpc_TestServer ^
--descriptor_set_out=NanoRpc_TestServer\app_test.protobuf


