#define _WINSOCKAPI_
#include <process.h>

#include <map>
#include <queue>
#include <chrono>
#include <thread>

#include "nanorpc/nanorpc2.h"
#include "nanorpc/winsock_channel.h"

#include "service.h"

namespace hw = ::hello_world;

void ClientThread() {
  auto channel = std::make_unique<nanorpc2::WinsockClientChannel>("localhost", "50372");
  nanorpc2::Client client(std::move(channel));
  
  // TODO: Patch proxy generator to take nanorpc2::Client instead of IRpcClient
  // TODO: Should it accept client at all? Should that be a shared pointer?
  // TODO: What happens if multiple threads call on the same proxy/client?
  // We definitely do not want to create a separate client for each thread,
  // because this would require new connection for each thread and the server
  // should be able to handle it.
  // The problems are as following:
  //    - There may be a policy to restrict a server to accept a single connection.
  //    - It is more complicate to handle on the server. In a simple application
  //      there is not reason for the server to handle more than one connection.
  //      This also makes it easier such that service does not have to be
  //      thread safe, even when not using call serializer.
  hw::OrderDesk_Proxy order_desk(&client);
  int32_t order_id = order_desk.CreateOrder(hw::DrinkType::Coffee, hw::ReadingType::Newspaper);

  while (!order_desk.IsOrderReady(order_id)) {
    Sleep(1000);
  }

  order_desk.GetDrink(order_id);
  order_desk.GetReading(order_id);

  client.Disconnect();
}

void ServerThread() {
  auto channel = std::make_unique<nanorpc2::WinsockServerChannel>("50372");
  nanorpc2::Server server(std::move(channel));

  OrderDeskImpl order_desk_service;
  hw::OrderDesk_Stub order_desk_service_stub(nullptr, &order_desk_service);
  server.RegisterService(&order_desk_service_stub);

  server.ConnectAndWait();
}

int main() {
  std::thread server_thread(ServerThread);
  server_thread.join();
  return 0;
}
