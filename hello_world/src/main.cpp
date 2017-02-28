#define _WINSOCKAPI_
#include <process.h>

#include <iostream>
#include <map>
#include <queue>
#include <chrono>
#include <thread>

#include "nanorpc/nanorpc2.h"
#include "nanorpc/client.h"
#include "nanorpc/server.h"
#include "nanorpc/winsock_channel.h"

#include "service.h"

namespace hw = ::hello_world;

class OrderDeskEventsListener : public hw::OrderDeskEvents {
public:
  OrderDeskEventsListener(hw::OrderDesk_Proxy *order_desk, int *orders_done)
      : order_desk_(order_desk), orders_done_(orders_done) {}

  void OrderStatusChanged(nanorpc::ServerContextInterface *context,
                          uint32_t order_id,
                          bool is_ready,
                          bool reading_taken,
                          bool drink_taken) override {
    std::cout << "Order status changed event for order " << order_id << "." << std::endl;
    std::cout << "Getting drink..." << std::endl;
    order_desk_->GetDrink(order_id);
    std::cout << "Getting reading..." << std::endl;
    order_desk_->GetReading(order_id);
    ++*orders_done_;
  }

private:
  hw::OrderDesk_Proxy *order_desk_;
  int *orders_done_;
};

void ClientThread() {
  std::cout << "Client thread started." << std::endl;

  auto channel = std::make_unique<nanorpc::WinsockClientChannel>("localhost", "50372");
  nanorpc::Client client(std::move(channel));

  // TODO: Patch proxy generator to take nanorpc2::Client instead of IRpcClient
  // TODO: Should it accept client at all? Should that be a shared pointer?
  // TODO: What happens if multiple threads call on the same proxy/client?
  // We definitely do not want to create a separate client for each thread,
  // because this requires a new connection for each thread and the server
  // should be able to handle it.
  // The problems are as following:
  //    - There may be a policy to restrict a server to accept a single connection.
  //    - It is more complex to handle on the server. In a simple application
  //      there is not reason for the server to handle more than one connection.
  //      This also makes it easier such that service does not have to be
  //      thread safe, even when not using call serializer.
  hw::OrderDesk_Proxy order_desk(&client);

  int orders_done = 0;
  OrderDeskEventsListener event_listener(&order_desk, &orders_done);
  hw::OrderDeskEvents_Stub event_listener_stub(&event_listener, nullptr);
  client.StartListening(&event_listener_stub);

  std::cout << "Client ready." << std::endl;

  for (int i = 0; i < 10; ++i) {
    std::cout << "Client creates order." << std::endl;
    order_desk.CreateOrder(hw::DrinkType::Coffee, hw::ReadingType::Newspaper);
  }

  while (orders_done < 10) {
    if (!client.PumpEvents(nullptr))
      std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  client.StopListening(event_listener_stub.GetInterfaceName());

  std::cout << "Client disconnecting." << std::endl;
  client.Shutdown();
  std::cout << "Client exiting." << std::endl;
}

void ServerThread() {
  std::cout << "Server thread started." << std::endl;
  auto transport = std::make_unique<nanorpc::WinsockServerTransport>("50372");
  nanorpc::SimpleServer server(std::move(transport));
  std::cout << "Server created." << std::endl;

  OrderDeskImpl order_desk_service(server.GetEventSource());
  hw::OrderDesk_Stub order_desk_service_stub(&order_desk_service, nullptr);
  server.RegisterService(&order_desk_service_stub);

  std::cout << "Server connecting..." << std::endl;
  server.ConnectAndWait();
  std::cout << "Server closed." << std::endl;
}

int main() {
  std::thread server_thread(ServerThread);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::thread client_thread(ClientThread);
  client_thread.join();
  server_thread.join();
  return 0;
}
