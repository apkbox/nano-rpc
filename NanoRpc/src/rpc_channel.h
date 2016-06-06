#if !defined(NANO_RPC_RPC_CHANNEL_HPP__)
#define NANO_RPC_RPC_CHANNEL_HPP__

#include <vector>

namespace nanorpc2 {

class RpcChannel {
public:
  virtual ~RpcChannel() {}

  virtual bool Connect() = 0;
  virtual void Close() = 0;

  virtual void Send(const void *message, size_t bytes) = 0;
  virtual bool ReceiveNonBlocking(void *message, size_t *bytes) = 0;
  virtual bool Receive(void *message, size_t *bytes) = 0;
};

class ChannelBuilder {
public:
  virtual ~ChannelBuilder() {}

  virtual RpcChannel *BuildServerChannel() = 0;
  virtual RpcChannel *BuildClientChannel() = 0;

protected:
  ChannelBuilder() {}
};

}  // namespace

#endif  // NANO_RPC_RPC_CHANNEL_HPP__
