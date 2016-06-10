#include "nanorpc/winsock_channel_impl.h"

#include <array>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "Ws2_32.lib")

namespace nanorpc2 {

WinsockServerChannel::Impl::Impl(const std::string &port)
    : port_(port),
      status_(ChannelStatus::NotConnected),
      socket_(INVALID_SOCKET),
      // TODO: Create event only on connect
      shutdown_event_(nullptr),
      socket_event_(nullptr) {}

WinsockServerChannel::Impl::~Impl() {
  Disconnect();
}

ChannelStatus WinsockServerChannel::Impl::GetStatus() const {
  return status_;
}

bool WinsockServerChannel::Impl::Connect() {
  if (status_ != ChannelStatus::NotConnected)
    return false;

  status_ = ChannelStatus::Connecting;

  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    status_ = ChannelStatus::NotConnected;
    return false;
  }

  struct addrinfo hints = {};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  // Resolve the local address and port to be used by the server
  struct addrinfo *local_address = nullptr;
  if (getaddrinfo(nullptr, port_.c_str(), &hints, &local_address) != 0) {
    WSACleanup();
    status_ = ChannelStatus::NotConnected;
    return false;
  }

  // Create a SOCKET for the server to listen for client connections
  socket_ = socket(local_address->ai_family, local_address->ai_socktype,
                   local_address->ai_protocol);
  if (socket_ == INVALID_SOCKET) {
    freeaddrinfo(local_address);
    WSACleanup();
    status_ = ChannelStatus::NotConnected;
    return false;
  }

  // Setup the TCP listening socket
  int result = bind(socket_, local_address->ai_addr,
                    static_cast<int>(local_address->ai_addrlen));
  if (result == SOCKET_ERROR) {
    freeaddrinfo(local_address);
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
    WSACleanup();
    status_ = ChannelStatus::NotConnected;
    return false;
  }

  freeaddrinfo(local_address);

  socket_event_ = WSACreateEvent();
  if (socket_event_ == WSA_INVALID_EVENT) {
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
    WSACleanup();
    status_ = ChannelStatus::NotConnected;
    return false;
  }

  shutdown_event_ = CreateEvent(nullptr, false, false, nullptr);
  if (shutdown_event_ == nullptr) {
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
    WSACloseEvent(socket_event_);
    WSACleanup();
    status_ = ChannelStatus::NotConnected;
    return false;
  }

  result = WSAEventSelect(socket_, socket_event_, FD_ACCEPT | FD_CLOSE);
  if (result == SOCKET_ERROR) {
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
    WSACloseEvent(socket_event_);
    socket_event_ = WSA_INVALID_EVENT;
    CloseHandle(shutdown_event_);
    shutdown_event_ = nullptr;
    WSACleanup();
    status_ = ChannelStatus::NotConnected;
    return false;
  }

  if (listen(socket_, SOMAXCONN) == SOCKET_ERROR) {
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
    WSACloseEvent(socket_event_);
    socket_event_ = WSA_INVALID_EVENT;
    CloseHandle(shutdown_event_);
    shutdown_event_ = nullptr;
    WSACleanup();
    status_ = ChannelStatus::NotConnected;
    return false;
  }

  SOCKET accept_socket = accept(socket_, nullptr, nullptr);
  if (socket_ == INVALID_SOCKET) {
    if (WSAGetLastError() != WSAEWOULDBLOCK) {
      closesocket(socket_);
      socket_ = INVALID_SOCKET;
      WSACloseEvent(socket_event_);
      socket_event_ = WSA_INVALID_EVENT;
      CloseHandle(shutdown_event_);
      shutdown_event_ = nullptr;
      WSACleanup();
      status_ = ChannelStatus::NotConnected;
      return false;
    }
  }

  std::array<HANDLE, 2> wait_handles = { socket_event_, shutdown_event_ };
  DWORD wait_result = WaitForMultipleObjects(
      wait_handles.size(), &wait_handles.front(), false, INFINITE);
  if (wait_result == (WAIT_OBJECT_0 + 1) || wait_result >= WAIT_ABANDONED ||
      wait_result == WAIT_TIMEOUT || wait_result == WAIT_FAILED) {
    // No longer need server socket
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
    WSACloseEvent(socket_event_);
    socket_event_ = WSA_INVALID_EVENT;
    CloseHandle(shutdown_event_);
    shutdown_event_ = nullptr;
    WSACleanup();
    status_ = ChannelStatus::NotConnected;
    return false;
  }

  accept_socket = accept(socket_, nullptr, nullptr);
  if (accept_socket == INVALID_SOCKET) {
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
    WSACloseEvent(socket_event_);
    socket_event_ = WSA_INVALID_EVENT;
    CloseHandle(shutdown_event_);
    shutdown_event_ = nullptr;
    WSACleanup();
    status_ = ChannelStatus::NotConnected;
    return false;
  }

  closesocket(socket_);
  socket_ = accept_socket;
  status_ = ChannelStatus::Established;

  return true;
}

void WinsockServerChannel::Impl::Disconnect() {
  if (status_ == ChannelStatus::NotConnected)
    return;

  if (socket_ != INVALID_SOCKET) {
    shutdown(socket_, SD_BOTH);
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
  }

  SetEvent(shutdown_event_);

  status_ = ChannelStatus::NotConnected;
}

}  // namespace nanorpc2
