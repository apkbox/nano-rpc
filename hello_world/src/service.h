#if !defined(HELLO_WORLD_SERVICE_H__)
#define HELLO_WORLD_SERVICE_H__

#include <atomic>
#include <mutex>
#include <thread>

#include "hello_world.nanorpc.pb.h"

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
  OrderDeskImpl()
      : last_waiting_number_(0),
        server_(std::thread(&OrderDeskImpl::ServerThread, this)) {}

  int32_t CreateOrder(hello_world::DrinkType drink,
                      hello_world::ReadingType reading) override {
    Order order(drink, reading);
    std::lock_guard<std::mutex> lock(mtx_);
    order.waiting_number_ = ++last_waiting_number_;
    pending_orders_.push(order);
    std::cout << "OrderDeskImpl: Creating order " << order.waiting_number_ << "." << std::endl;

    return order.waiting_number_;
  }

  bool IsOrderReady(int order_number) override {
    std::cout << "OrderDeskImpl: Asked 'is order ready?'." << std::endl;
    std::lock_guard<std::mutex> lock(mtx_);
    return completed_orders_.find(order_number) != completed_orders_.end();
  }

  hello_world::DrinkType GetDrink(int order_number) override {
    std::cout << "OrderDeskImpl: Getting drink." << std::endl;
    std::lock_guard<std::mutex> lock(mtx_);
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
    std::cout << "OrderDeskImpl: Getting reading." << std::endl;
    std::lock_guard<std::mutex> lock(mtx_);
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
    std::lock_guard<std::mutex> lock(mtx_);
    if (iter->second.drink_taken_ && iter->second.reading_taken_)
      completed_orders_.erase(iter->first);
  }

  void ServerThread() {
    std::cout << "OrderDeskImpl: ...and serving." << std::endl;
    std::lock_guard<std::mutex> lock(mtx_);
    Order next = pending_orders_.front();
    completed_orders_[next.waiting_number_] = next;
    pending_orders_.pop();
  }

  int last_waiting_number_;
  std::queue<Order> pending_orders_;
  std::map<int, Order> completed_orders_;

  std::mutex mtx_;
  std::thread server_;
};

#endif  // HELLO_WORLD_SERVICE_H__
