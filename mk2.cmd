@echo off
set BASE_PATH=%~dp0
%BASE_PATH%\third_party\protobuf\bin\protoc ^
--proto_path=%BASE_PATH%\nanorpc\src\ ^
%BASE_PATH%\nanorpc\src\nanorpc\rpc_types.proto ^
--cpp_out=%BASE_PATH%\nanorpc\src\ ^
--js_out=import_style=commonjs,binary:%BASE_PATH%\nanorpc\src\

%BASE_PATH%\third_party\protobuf\bin\protoc ^
--proto_path=%BASE_PATH%\nanorpc\src\ ^
--proto_path=%BASE_PATH%\arizona_api_server\src\ ^
%BASE_PATH%\arizona_api_server\src\arizona_api_server.proto ^
--cpp_out=%BASE_PATH%\arizona_api_server\src\ ^
--js_out=import_style=commonjs,binary:%BASE_PATH%\arizona_api_server\src\ ^
--plugin=protoc-gen-nanorpc_cpp=%BASE_PATH%\bin\x64\Debug\nanorpc_cpp_plugin.exe ^
--nanorpc_cpp_out=%BASE_PATH%\arizona_api_server\src\
