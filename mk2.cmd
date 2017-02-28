@echo off
D:\Projects\nano-rpc\third_party\protobuf\bin\protoc ^
--proto_path=D:\Projects\nano-rpc\nanorpc\src\ ^
D:\Projects\nano-rpc\nanorpc\src\nanorpc\rpc_types.proto ^
--cpp_out=D:\Projects\nano-rpc\nanorpc\src\ ^
--js_out=import_style=commonjs,binary:D:\Projects\nano-rpc\nanorpc\src\

D:\Projects\nano-rpc\third_party\protobuf\bin\protoc ^
--proto_path=D:\Projects\nano-rpc\nanorpc\src\ ^
--proto_path=D:\Projects\nano-rpc\arizona_api_server\src\ ^
D:\Projects\nano-rpc\arizona_api_server\src\arizona_api_server.proto ^
--cpp_out=D:\Projects\nano-rpc\arizona_api_server\src\ ^
--js_out=import_style=commonjs,binary:D:\Projects\nano-rpc\arizona_api_server\src\ ^
--plugin=protoc-gen-nanorpc_cpp=D:\Projects\nano-rpc\bin\x64\Debug\nanorpc_cpp_plugin.exe ^
--nanorpc_cpp_out=D:\Projects\nano-rpc\arizona_api_server\src\
