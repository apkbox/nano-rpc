#if !defined(NANO_RPC_RPC_CHANNEL_HPP__)
#define NANO_RPC_RPC_CHANNEL_HPP__

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

}  // namespace

#endif  // NANO_RPC_RPC_CHANNEL_HPP__
