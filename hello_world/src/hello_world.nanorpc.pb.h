// Generated by the nanorpc protobuf plugin.
// If you make any local change, they will be lost.
// source: hello_world.proto
#ifndef NANORPC_hello_5fworld_2eproto__INCLUDED
#define NANORPC_hello_5fworld_2eproto__INCLUDED

#include "hello_world.pb.h"
#include "nanorpc/rpc_client.hpp"
#include "nanorpc/rpc_stub.hpp"
#include "nanorpc/rpc_object_manager.hpp"

namespace hello_world {

class OrderDesk {
public:
  virtual ~OrderDesk() {}

  virtual int32_t CreateOrder(DrinkType drink, ReadingType reading) = 0;
  virtual bool IsOrderReady(int32_t value) = 0;
  virtual DrinkType GetDrink(int32_t value) = 0;
  virtual ReadingType GetReading(int32_t value) = 0;
};

class OrderDesk_Stub : public nanorpc::IRpcStub {
public:
  explicit OrderDesk_Stub(nanorpc::IRpcObjectManager* object_manager, OrderDesk* impl)
      : object_manager_(object_manager), impl_(impl) {}

  const char *GetInterfaceName() const;
  void CallMethod(const nanorpc::RpcCall &rpc_call, nanorpc::RpcResult *rpc_result);

private:
  nanorpc::IRpcObjectManager* object_manager_;
  OrderDesk* impl_;
};

class OrderDesk_Proxy : public OrderDesk {
public:
  explicit OrderDesk_Proxy(nanorpc::IRpcClient *client, nanorpc::RpcObjectId object_id = 0)
      : client_(client), object_id_(object_id) {}

  virtual ~OrderDesk_Proxy();

  int32_t CreateOrder(DrinkType drink, ReadingType reading) override;
  bool IsOrderReady(int32_t value) override;
  DrinkType GetDrink(int32_t value) override;
  ReadingType GetReading(int32_t value) override;

private:
  nanorpc::IRpcClient *client_;
  nanorpc::RpcObjectId object_id_;
};

}  // namespace hello_world


#endif  // NANORPC_hello_5fworld_2eproto__INCLUDED