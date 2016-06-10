#define _WINSOCKAPI_
#include <process.h>

#include <map>
#include <queue>
#include <chrono>
#include <thread>

#include "nanorpc\rpc_client.hpp"
#include "nanorpc\rpc_stub.hpp"
#include "nanorpc\rpc_server.h"
#include "nanorpc\server_builder.h"
#include "nanorpc\rpc_client.hpp"
#include "nanorpc\rpc_object_manager.hpp"
#include "nanorpc\named_pipe_rpc_channel.h"
#include "hello_world.nanorpc.pb.h"

#include "nanorpc/nanorpc2.h"
#include "nanorpc/winsock_channel.h"

struct Order {
  Order()
      : drink_(hello_world::NoDrink),
        reading_(hello_world::NoReading),
        drink_taken_(false),
        reading_taken_(false),
  time_to_complete_(-1) {}

  Order(hello_world::DrinkType drink, hello_world::ReadingType reading)
      : drink_(drink), reading_(reading), time_to_complete_(-1) {}

  hello_world::DrinkType drink_;
  hello_world::ReadingType reading_;
  bool drink_taken_;
  bool reading_taken_;
  int time_to_complete_;
  int waiting_number_;
};

class OrderDeskImpl : public hello_world::OrderDesk {
public:
  OrderDeskImpl() : last_waiting_number_(0) {}

  int32_t CreateOrder(hello_world::DrinkType drink, hello_world::ReadingType reading) override {
    Order order(drink, reading);
    order.waiting_number_ = ++last_waiting_number_;
    pending_orders_.push(order);
  }

  bool IsOrderReady(int order_number) override {
    return completed_orders_.find(order_number) != completed_orders_.end();
  }

  hello_world::DrinkType GetDrink(int order_number) override {
    auto &order = completed_orders_.find(order_number);
    if (order != completed_orders_.end() && !order->second.drink_taken_) {
      order->second.drink_taken_ = true;
      hello_world::DrinkType d = order->second.drink_;
      RemoveIfFilled(order);
      return d;
    }

    return hello_world::NoDrink;
  }

  hello_world::ReadingType GetReading(int order_number) override {
    auto &order = completed_orders_.find(order_number);
    if (order != completed_orders_.end() && !order->second.reading_taken_) {
      order->second.reading_taken_ = true;
      hello_world::ReadingType r = order->second.reading_;
      RemoveIfFilled(order);
      return r;
    }

    return hello_world::NoReading;
  }

private:
  void RemoveIfFilled(const std::map<int, Order>::iterator &iter) {
    if (iter->second.drink_taken_ && iter->second.reading_taken_)
      completed_orders_.erase(iter->first);
  }

  int last_waiting_number_;
  std::queue<Order> pending_orders_;
  std::map<int, Order> completed_orders_;
};


static void server_thread(nanorpc2::WinsockServerChannel *channel) {
  std::this_thread::sleep_for(std::chrono::seconds(3));
  channel->Disconnect();

  /*
  OrderDeskImpl order_desk_service;
  std::vector<std::unique_ptr<nanorpc::RpcServer>> servers;

  while (true) {
    nanorpc::NamedPipeChannel ch(L"hello_world");
    nanorpc::ServerBuilder server_builder;
    server_builder.set_channel_builder(&ch);
    server_builder.RegisterService(hello_world::OrderDesk_Stub(&order_desk_service));
    std::unique_ptr<nanorpc::RpcServer> server(server_builder.Build());
    servers.emplace_back(server);
  }
  */
}


int main() {
  nanorpc2::WinsockServerChannel ch;
  std::thread thread(server_thread, &ch);
  ch.Connect("99344");
  std::this_thread::sleep_for(std::chrono::seconds(3));
  thread.join();

  return 0;
}

