#if !defined(NANORPC_WINSOCK_CHANNEL_IMPL_H__)
#define NANORPC_WINSOCK_CHANNEL_IMPL_H__

#include <cassert>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include <winsock2.h>

#include "nanorpc/nanorpc2.h"
#include "nanorpc/winsock_channel.h"

namespace nanorpc {

class WinsockServerChannelImpl : public ServerChannelInterface {
  friend class WinsockServerTransportImpl;
public:
  ~WinsockServerChannelImpl();

  ChannelStatus GetStatus() const override { return status_; }

  bool Connect() override { assert(false); return false; }
  void Shutdown() override;
  void Disconnect() override;

  std::unique_ptr<ReadBuffer> Read(size_t bytes) override;
  std::unique_ptr<WriteBuffer> CreateWriteBuffer() override;
  void Write(std::unique_ptr<WriteBuffer> buffer) override;

  bool Read(void *buffer, size_t buffer_size, size_t *bytes_read);
  bool Write(const void *buffer, size_t buffer_size);

private:
  explicit WinsockServerChannelImpl(SOCKET socket);

  void Cleanup();

  ChannelStatus status_;

  SOCKET socket_;

  std::mutex read_lock_;
  std::mutex write_lock_;

  NANORPC_DISALLOW_COPY_AND_ASSIGN(WinsockServerChannelImpl);
};

class WinsockChannelImpl {
public:
  explicit WinsockChannelImpl(const std::string &port);
  explicit WinsockChannelImpl(const std::string &address, const std::string &port);
  ~WinsockChannelImpl();

  ChannelStatus GetStatus() const;

  bool Connect();
  void Shutdown();
  void Disconnect();

  std::unique_ptr<ReadBuffer> Read(size_t bytes);
  std::unique_ptr<WriteBuffer> CreateWriteBuffer();
  void Write(std::unique_ptr<WriteBuffer> buffer);

  bool Read(void *buffer, size_t buffer_size, size_t *bytes_read);
  bool Write(const void *buffer, size_t buffer_size);

private:
  bool ConnectServer();
  bool ConnectClient();

  void Cleanup();

  std::string address_;
  std::string port_;
  int connect_timeout_;
  const bool is_client;

  ChannelStatus status_;

  struct addrinfo *addrinfo_;
  SOCKET listening_socket_;
  SOCKET socket_;

  std::mutex read_lock_;
  std::mutex write_lock_;

  NANORPC_DISALLOW_COPY_AND_ASSIGN(WinsockChannelImpl);
};


}  // namespace nanorpc

#endif  // NANORPC_WINSOCK_CHANNEL_IMPL_H__
