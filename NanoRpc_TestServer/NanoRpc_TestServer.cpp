#include <memory>
#include <iostream>
#include "nano_rpc.hpp"
#include "named_pipe_connector.h"
#include "named_pipe_rpc_channel.h"

#if  0
class ServerBuilder {
public:
  ServerBuilder() {}
  void SetChannel(NanoRpc::RpcChannel &channel);
  void RegisterService(NanoRpc::IRpcStub *service);
  NanoRpc::RpcServer *Build();
};
#endif


int wmain(int argc, wchar_t *argv[]) { 
  nanorpc2::NamedPipeRpcChannelBuilder cb(L"test-channel");
  cb.set_client_side(false);
  std::unique_ptr<nanorpc2::NamedPipeRpcChannel> ch(cb.Build());
  if (ch->Connect())
    std::cout << "Connected..." << std::endl;

#if 0
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
#endif

  return 0;
}
