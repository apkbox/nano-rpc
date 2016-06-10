#include <memory>
#include <thread>
#include <sstream>
#include <chrono>
#include <iostream>

#include "nanorpc/nano_rpc.h"
#include "nanorpc/named_pipe_connector.h"
#include "nanorpc/named_pipe_rpc_channel.h"

#include <Windows.h>

bool g_server_shutdown = false;
bool g_client_finished = false;
bool g_server_finished = false;

void server_thread_proc() {
  nanorpc::NamedPipeChannel channel(L"test-channel");
  std::unique_ptr<nanorpc::RpcChannel> ch(channel.BuildServerChannel());
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

  std::cout << "SERVER: Exiting." << std::endl;
  g_server_finished = true;
}

void client_thread_proc() {
  nanorpc::NamedPipeChannel cb(L"test-channel");
  std::unique_ptr<nanorpc::RpcChannel> ch(cb.BuildClientChannel());
  std::cout << "CLIENT: Connecting..." << std::endl;
  if (ch->Connect())
    std::cout << "CLIENT: Connected." << std::endl;

  const int kNumberOfRequests = 10;

  int i = 0;
  while (!g_server_shutdown && (i++ < kNumberOfRequests)) {
    std::ostringstream s;
    s << "Ping " << i++ << std::endl;
    auto str = s.str();
    ch->Send(str.c_str(), str.length() + 1);
    Sleep(100);
  }

  std::cout << "CLIENT: Exiting." << std::endl;
  g_client_finished = true;
}

int wmain(int argc, wchar_t *argv[]) {
  std::thread server_thread(server_thread_proc);
  std::thread client_thread(client_thread_proc);

  while (!g_server_shutdown && !g_client_finished && !g_server_finished) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  g_server_shutdown = true;

  std::cout << "MASTER: Exiting." << std::endl;

  client_thread.join();
  server_thread.join();

  return 0;
}
