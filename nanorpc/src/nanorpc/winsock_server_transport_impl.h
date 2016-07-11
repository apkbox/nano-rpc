#if !defined(NANORPC_WINSOCK_SERVER_TRANSPORT_IMPL_H__)
#define NANORPC_WINSOCK_SERVER_TRANSPORT_IMPL_H__

#include <memory>
#include <mutex>
#include <string>

#include <winsock2.h>

#include "nanorpc/nanorpc2.h"
#include "nanorpc/winsock_channel.h"

namespace nanorpc {

class WinsockServerChannelImpl;

class WinsockServerTransportImpl {
public:
  explicit WinsockServerTransportImpl(const std::string &port);
  ~WinsockServerTransportImpl();

  bool IsBound() const;

  std::unique_ptr<WinsockServerChannelImpl> Listen();
  void Cancel();

private:
  void Cleanup();

  std::string port_;

  std::mutex binding_mtx_;
  bool wsa_started_;
  bool is_bound_;

  struct addrinfo *addrinfo_;
  SOCKET listening_socket_;

  NANORPC_DISALLOW_COPY_AND_ASSIGN(WinsockServerTransportImpl);
};

}  // namespace nanorpc

#endif  // NANORPC_WINSOCK_SERVER_TRANSPORT_IMPL_H__
