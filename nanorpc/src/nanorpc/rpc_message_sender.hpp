#if !defined(NANO_RPC_RPC_MESSAGE_SENDER_HPP__)
#define NANO_RPC_RPC_MESSAGE_SENDER_HPP__

#include "nanorpc/rpc_types.pb.h"

namespace nanorpc {

class IRpcMessageSender {
public:
  virtual ~IRpcMessageSender() {}

  virtual void Send(RpcMessage &rpcMessage) = 0;
};

} // namespace

#endif // NANO_RPC_RPC_MESSAGE_SENDER_HPP__