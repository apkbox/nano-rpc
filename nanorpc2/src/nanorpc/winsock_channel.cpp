#include "nanorpc/winsock_channel.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <array>

#define _WINSOCKAPI_
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include "nanorpc/nanorpc2.h"

#pragma comment(lib, "Ws2_32.lib")

namespace nanorpc2 {

WinsockServerChannel::WinsockServerChannel()
    : status_(ChannelStatus::NotConnected),
      socket_(0),
      shutdown_event_(nullptr) {
  shutdown_event_ = CreateEvent(nullptr, false, false, nullptr);
}

bool WinsockServerChannel::WaitForClient() {
  return false;
}

bool WinsockServerChannel::WaitForClientAsync() {
  return false;
}

void WinsockServerChannel::Disconnect() {
  if (status_ == ChannelStatus::NotConnected)
    return;

  if (socket_ != 0) {
    closesocket(socket_);
    socket_ = 0;
  }

  SetEvent(shutdown_event_);
  status_ = ChannelStatus::NotConnected;
}

bool WinsockServerChannel::Read(void *buffer,
                                size_t buffer_size,
                                size_t *bytes_read) {
  return false;
}

bool WinsockServerChannel::Write(void *buffer, size_t buffer_size) {
  return false;
}

bool WinsockServerChannel::Connect(const std::string &port) {
  WSADATA wsaData;
  int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (result != 0) {
    throw std::exception("failed to initialize windows sockets");
    return false;
  }

  struct addrinfo *local_address = nullptr;
  struct addrinfo hints;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  // Resolve the local address and port to be used by the server
  result = getaddrinfo(nullptr, port.c_str(), &hints, &local_address);
  if (result != 0) {
    WSACleanup();
    throw std::exception("getaddrinfo failed");
    return false;
  }

  // Create a SOCKET for the server to listen for client connections
  SOCKET listening_socket =
      socket(local_address->ai_family, local_address->ai_socktype,
             local_address->ai_protocol);
  if (listening_socket == INVALID_SOCKET) {
    freeaddrinfo(local_address);
    WSACleanup();
    throw std::exception("Error at socket()");
    return false;
  }

  // Setup the TCP listening socket
  result = bind(listening_socket, local_address->ai_addr,
                (int)local_address->ai_addrlen);
  if (result == SOCKET_ERROR) {
    freeaddrinfo(local_address);
    closesocket(listening_socket);
    WSACleanup();
    throw std::exception("bind failed with error");
    return false;
  }

  freeaddrinfo(local_address);

  HANDLE accept_event = WSACreateEvent();
  WSAEventSelect(listening_socket, accept_event, FD_ACCEPT | FD_CLOSE);

  if (listen(listening_socket, SOMAXCONN) == SOCKET_ERROR) {
    closesocket(listening_socket);
    WSACleanup();
    throw std::exception("Listen failed with error");
    return false;
  }

  socket_ = accept(listening_socket, nullptr, nullptr);
  if (socket_ == INVALID_SOCKET) {
    if (WSAGetLastError() != WSAEWOULDBLOCK) {
      char buffer[1024];
      _snprintf_c(buffer, 1024, "accept failed with error %i",
                  WSAGetLastError());
      OutputDebugStringA(buffer);
      closesocket(listening_socket);
      WSACleanup();
      return false;
    }
  }

  status_ = ChannelStatus::Connecting;
  std::array<HANDLE, 2> wait_handles = { accept_event, shutdown_event_ };
  DWORD wait_result = WaitForMultipleObjects(
      wait_handles.size(), &wait_handles.front(), false, INFINITE);
  if (wait_result == WAIT_OBJECT_0) {
    socket_ = accept(listening_socket, nullptr, nullptr);
    if (socket_ == INVALID_SOCKET) {
      closesocket(listening_socket);
      WSACleanup();
      status_ = ChannelStatus::NotConnected;
      return false;
    }

    status_ = ChannelStatus::Established;
    closesocket(listening_socket);
    // READY HERE
  } else {
    // No longer need server socket
    closesocket(listening_socket);
    closesocket(socket_);
    WSACleanup();
  }

  return true;
}

ChannelStatus WinsockServerChannel::GetStatus() const {
  return status_;
}

}  // namespace nanorpc2
