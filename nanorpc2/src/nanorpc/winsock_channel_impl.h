#if !defined(NANORPC_WINSOCK_CHANNEL_IMPL_H__)
#define NANORPC_WINSOCK_CHANNEL_IMPL_H__

#include <atomic>
#include <string>

#include <winsock2.h>

#include "nanorpc/nanorpc2.h"
#include "nanorpc/winsock_channel.h"

namespace nanorpc2 {

class WinsockServerChannel::Impl {
public:
  Impl() {}
  explicit Impl(const std::string &port);
  ~Impl();

  ChannelStatus GetStatus() const;

  bool Connect();
  void Disconnect();

private:
  std::string port_;
  std::atomic<ChannelStatus> status_;
  HANDLE shutdown_event_;
  WSAEVENT socket_event_;
  SOCKET socket_;

  NANORPC_DISALLOW_COPY_AND_ASSIGN(WinsockServerChannel::Impl);
};

}  // namespace nanorpc2

#endif  // NANORPC_WINSOCK_CHANNEL_IMPL_H__
