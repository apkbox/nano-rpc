#include "nano_rpc.hpp"
#include "named_pipe_connector.hpp"
#include "named_pipe_rpc_channel.hpp"


int wmain(int argc, wchar_t *argv[]) { 
  NanoRpc::NamedPipeRpcChannel ch("test-channel", ".");
  ch.set_connect_timeout(-1);

  NanoRpc::RpcServer server(ch);
  server.RegisterService(new MyService());
  server.Run();   // Run until connection is closed

  // server.WaitForRequest



  return 0;
}
