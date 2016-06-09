#include <memory>
#include <thread>
#include <sstream>
#include <chrono>
#include <iostream>
#include "nanorpc/server_builder.h"
#include "nanorpc/named_pipe_connector.h"
#include "nanorpc/named_pipe_rpc_channel.h"

#include <Windows.h>

bool g_server_shutdown = false;

class MyService : public nanorpc::IRpcService {
public:
  void CallMethod(const nanorpc::RpcCall &rpc_call, nanorpc::RpcResult *rpc_result) override {

  }
};

void server_thread_proc() {
  nanorpc::NamedPipeChannel ch(L"test-channel");
  nanorpc::ServerBuilder sb;
  sb.set_channel_builder(&ch);
  sb.RegisterService("MyService", new MyService());

  std::unique_ptr<nanorpc::RpcServer> server(sb.Build());
  std::cout << "SERVER: Connecting..." << std::endl;
  if (server->Start())
    std::cout << "SERVER: Connected." << std::endl;
}

void client_thread_proc() {
#if 0
  nanorpc::NamedPipeRpcChannelBuilder cb(L"test-channel");
  cb.set_client_side(true);
  std::unique_ptr<nanorpc::NamedPipeRpcChannel> ch(cb.Build());
  std::cout << "CLIENT: Connecting..." << std::endl;
  if (ch->Connect())
    std::cout << "CLIENT: Connected." << std::endl;

  int i = 0;
  while (!g_server_shutdown) {
    std::ostringstream s;
    s << "Ping " << i++ << std::endl;
    auto str = s.str();
    ch->Send(str.c_str(), str.length() + 1);
    Sleep(100);
  }

  ch->Close();
#endif
}

int wmain(int argc, wchar_t *argv[]) {
  std::cout << "Press 'q' to exit." << std::endl;

  std::thread server_thread(server_thread_proc);
  std::thread client_thread(client_thread_proc);

  while (!g_server_shutdown) {
    char c;
    std::cin >> c;
    if (c == 'q')
      g_server_shutdown = true;
  }

  server_thread.join();
  client_thread.join();

  return 0;
}
