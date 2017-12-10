@echo off
set BASE_PATH=%~dp0
rem third_party\protobuf\bin\protoc rpc_proto\rpc_types.proto --cpp_out=NanoRpc\src
%BASE_PATH%\third_party\protobuf\bin\protoc codegen_test.proto ^
--descriptor_set_out=%BASE_PATH%\codegen_test\codegen_test.protobuf ^
--cpp_out=codegen_test ^
--plugin=protoc-gen-nanorpc_cpp=%BASE_PATH%\x64\Debug\nanorpc_cpp_plugin.exe --nanorpc_cpp_out=codegen_test


