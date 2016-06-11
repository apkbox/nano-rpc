#if !defined(NANORPC_WINSOCK_CHANNEL_IMPL_H__)
#define NANORPC_WINSOCK_CHANNEL_IMPL_H__

#include <atomic>
#include <string>

#include <winsock2.h>

#include "nanorpc/nanorpc2.h"
#include "nanorpc/winsock_channel.h"

namespace nanorpc2 {

class WinsockChannelImpl {
public:
  explicit WinsockChannelImpl(const std::string &port);
  explicit WinsockChannelImpl(const std::string &address, const std::string &port);
  ~WinsockChannelImpl();

  ChannelStatus GetStatus() const;

  bool Connect();
  void Disconnect();

  bool Read(void *buffer, size_t buffer_size, size_t *bytes_read);
  bool Write(void *buffer, size_t buffer_size);

private:
  bool ConnectServer();
  bool ConnectClient();

  void Cleanup();

  std::string address_;
  std::string port_;
  int connect_timeout_;
  const bool is_client;
  std::atomic<ChannelStatus> status_;

  struct addrinfo *addrinfo_;
  HANDLE shutdown_event_;
  WSAEVENT socket_event_;
  SOCKET listening_socket_;
  SOCKET socket_;

  NANORPC_DISALLOW_COPY_AND_ASSIGN(WinsockChannelImpl);
};

}  // namespace nanorpc2

#endif  // NANORPC_WINSOCK_CHANNEL_IMPL_H__
