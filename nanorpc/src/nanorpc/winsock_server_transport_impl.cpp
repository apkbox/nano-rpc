#include "nanorpc/winsock_channel_impl.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#pragma comment(lib, "Ws2_32.lib")

namespace nanorpc {

WinsockServerTransportImpl::WinsockServerTransportImpl(const std::string &port)
    : port_(port),
      is_bound_(false),
      wsa_started_(false),
      addrinfo_(nullptr),
      listening_socket_(INVALID_SOCKET) {}

WinsockServerTransportImpl::~WinsockServerTransportImpl() {
  Cancel();
}

bool WinsockServerTransportImpl::IsBound() const {
  return is_bound_;
}

std::unique_ptr<WinsockChannelImpl> WinsockServerTransportImpl::Listen() {
  std::unique_lock<std::mutex> lock(binding_mtx_);
  if (!is_bound_) {
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
      goto error;

    wsa_started_ = true;

    // Resolve the local address and port to be used by the server
    if (getaddrinfo(nullptr, port_.c_str(), &hints, &addrinfo_) != 0)
      goto error;

    // Create a SOCKET for the server to listen for client connections
    listening_socket_ =
      socket(addrinfo_->ai_family, addrinfo_->ai_socktype, addrinfo_->ai_protocol);
    if (listening_socket_ == INVALID_SOCKET)
      goto error;

    // Setup the TCP listening socket
    if (bind(listening_socket_, addrinfo_->ai_addr,
      static_cast<int>(addrinfo_->ai_addrlen)) == SOCKET_ERROR)
      goto error;

    is_bound_ = true;
  }

  lock.unlock();

  if (listen(listening_socket_, SOMAXCONN) == SOCKET_ERROR)
    goto error;

  SOCKET socket = accept(listening_socket_, nullptr, nullptr);
  if (socket == INVALID_SOCKET) {
    if (WSAGetLastError() != WSAEWOULDBLOCK)
      goto error;
  }

  return std::unique_ptr<WinsockChannelImpl>{new WinsockChannelImpl{socket}};

error:
  Cleanup();
  return nullptr;
}

void WinsockServerTransportImpl::Cancel() {
  Cleanup();
}

void WinsockServerTransportImpl::Cleanup() {
  std::lock_guard<std::mutex> lock(binding_mtx_);

  if (addrinfo_ != nullptr) {
    freeaddrinfo(addrinfo_);
    addrinfo_ = nullptr;
  }

  if (listening_socket_ != INVALID_SOCKET) {
    closesocket(listening_socket_);
    listening_socket_ = INVALID_SOCKET;
  }

  if (wsa_started_) {
    wsa_started_ = false;
    WSACleanup();
  }

  is_bound_ = false;
}

}  // namespace nanorpc
