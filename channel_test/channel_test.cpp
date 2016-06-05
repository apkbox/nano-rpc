#include <memory>
#include <thread>
#include <sstream>
#include <chrono>
#include <iostream>
#include "nano_rpc.hpp"
#include "named_pipe_connector.h"
#include "named_pipe_rpc_channel.h"

#include <Windows.h>

bool g_server_shutdown = false;

void server_thread_proc() {
  nanorpc2::NamedPipeRpcChannelBuilder cb(L"test-channel");
  cb.set_client_side(false);
  std::unique_ptr<nanorpc2::NamedPipeRpcChannel> ch(cb.Build());
  std::cout << "SERVER: Connecting..." << std::endl;
  if (ch->Connect())
    std::cout << "SERVER: Connected." << std::endl;

  while (!g_server_shutdown) {
    std::cout << "Waiting..." << std::endl;
    char buffer[256];
    size_t bytes = sizeof(buffer);
    if (ch->Receive(buffer, &bytes)) {
      std::cout << "Received: " << buffer << std::endl;
    }
  }
}

void client_thread_proc() {
  nanorpc2::NamedPipeRpcChannelBuilder cb(L"test-channel");
  cb.set_client_side(true);
  std::unique_ptr<nanorpc2::NamedPipeRpcChannel> ch(cb.Build());
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
