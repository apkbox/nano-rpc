#include <memory>
#include "nano_rpc.hpp"
#include "named_pipe_connector.hpp"
#include "named_pipe_rpc_channel.hpp"

class ServerBuilder {
public:
  ServerBuilder() {}
  void SetChannel(NanoRpc::RpcChannel &channel);
  void RegisterService(NanoRpc::IRpcStub *service);
  NanoRpc::RpcServer *Build();
};


int wmain(int argc, wchar_t *argv[]) { 
  NanoRpc::NamedPipeRpcChannel ch("test-channel", ".");
  ch.set_connect_timeout(-1);
  ch.set_reconnect(true);

  ServerBuilder server_builder;
  server_builder.RegisterService(&ch);
  server_builder.RegisterService(new MyService());

  std::shared_ptr<NanoRpc::RpcServer> server(server_builder.Build());
  server->Run();   // Run until connection is closed

  // server->WaitForRequest();
  // server->Shutdown();


  return 0;
}
