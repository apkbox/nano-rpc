#if !defined(NANORPC_WINSOCK_CHANNEL_H__)
#define NANORPC_WINSOCK_CHANNEL_H__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define _WINSOCKAPI_
#include <winsock2.h>

#include "nanorpc/nanorpc2.h"

namespace nanorpc2 {

class WinsockServerChannel : public ServerChannelInterface {
public:
  WinsockServerChannel();

  ChannelStatus GetStatus() const override;

  bool WaitForClient() override;
  bool WaitForClientAsync() override;
  void Disconnect() override;

  bool Read(void *buffer, size_t buffer_size, size_t *bytes_read) override;
  bool Write(void *buffer, size_t buffer_size) override;

  // TODO: Make private
  bool Connect(const std::string &port);

private:
  WinsockServerChannel(const WinsockServerChannel &) = delete;
  WinsockServerChannel &operator=(const WinsockServerChannel &) = delete;

  ChannelStatus status_;

  // TODO: Hide implementation details.
  HANDLE shutdown_event_;
  SOCKET socket_;
};

}  // namespace nanorpc2

#endif  // NANORPC_WINSOCK_CHANNEL_H__
