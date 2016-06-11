#include "nanorpc/winsock_channel_impl.h"

#include <atomic>
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

WinsockChannelImpl::WinsockChannelImpl(const std::string &port)
    : address_(),
      port_(port),
      is_client(false),
      status_(ChannelStatus::NotConnected),
      addrinfo_(nullptr),
      shutdown_event_(nullptr),
      socket_event_(nullptr),
      listening_socket_(INVALID_SOCKET),
      socket_(INVALID_SOCKET) {}

WinsockChannelImpl::WinsockChannelImpl(const std::string &address,
                                       const std::string &port)
    : address_(address),
      port_(port),
      is_client(true),
      status_(ChannelStatus::NotConnected),
      addrinfo_(nullptr),
      shutdown_event_(nullptr),
      socket_event_(nullptr),
      listening_socket_(INVALID_SOCKET),
      socket_(INVALID_SOCKET) {}

WinsockChannelImpl::~WinsockChannelImpl() {
  Disconnect();
}

ChannelStatus WinsockChannelImpl::GetStatus() const {
  return status_;
}

bool WinsockChannelImpl::Connect() {
  if (is_client)
    return ConnectClient();
  else
    return ConnectServer();
}

void WinsockChannelImpl::Cleanup() {
  if (addrinfo_ != nullptr) {
    freeaddrinfo(addrinfo_);
    addrinfo_ = nullptr;
  }

  if (listening_socket_ != INVALID_SOCKET) {
    closesocket(listening_socket_);
    listening_socket_ = INVALID_SOCKET;
  }

  if (socket_ != INVALID_SOCKET) {
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
  }

  if (socket_event_ != WSA_INVALID_EVENT) {
    WSACloseEvent(socket_event_);
    socket_event_ = WSA_INVALID_EVENT;
  }

  if (shutdown_event_ != nullptr) {
    CloseHandle(shutdown_event_);
    shutdown_event_ = nullptr;
  }

  WSACleanup();
  status_.store(ChannelStatus::NotConnected);
}

bool WinsockChannelImpl::ConnectClient() {
  auto exp = ChannelStatus::NotConnected;
  if (!status_.compare_exchange_strong(exp, ChannelStatus::Connecting))
    return false;

  bool result = false;

  struct addrinfo hints = {};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    goto exit;

  // Resolve server address
  if (getaddrinfo(address_.c_str(), port_.c_str(), &hints, &addrinfo_) != 0)
    goto exit;

  socket_event_ = WSACreateEvent();
  if (socket_event_ == WSA_INVALID_EVENT)
    goto exit;

  bool connected = false;
  for (auto p = addrinfo_; p != nullptr; p = p->ai_next) {
    // Create client socket
    socket_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (socket_ == INVALID_SOCKET)
      continue;

    if (WSAEventSelect(socket_, socket_event_, FD_CONNECT | FD_CLOSE) != 0)
      goto exit;

    if (connect(socket_, p->ai_addr, static_cast<int>(p->ai_addrlen)) == 0) {
      connected = true;
      break;
    }

    if (WSAGetLastError() == WSAEWOULDBLOCK) {
      std::array<HANDLE, 2> wait_handles = { socket_event_, shutdown_event_ };
      DWORD wait_result = WSAWaitForMultipleEvents(
          wait_handles.size(), &wait_handles.front(), false, INFINITE, true);
      if (wait_result == WAIT_OBJECT_0) {
        WSANETWORKEVENTS events;
        if (WSAEnumNetworkEvents(socket_, socket_event_, &events) == 0) {
          if (events.lNetworkEvents & FD_CLOSE_BIT) {
            closesocket(socket_);
            socket_ = INVALID_SOCKET;
            continue;
          } else if (events.lNetworkEvents & FD_CONNECT_BIT) {
            connected = true;
            break;
          }
        }
      }
      if (wait_result == (WAIT_OBJECT_0 + 1)) {
        // Shutdown called.
        break;
      }
    }

    closesocket(socket_);
    socket_ = INVALID_SOCKET;
  }

  if (connected) {
    result = true;
    status_.store(ChannelStatus::Established);
  }

exit:
  if (!result)
    Cleanup();

  return result;
}

bool WinsockChannelImpl::ConnectServer() {
  auto exp = ChannelStatus::NotConnected;
  if (!status_.compare_exchange_strong(exp, ChannelStatus::Connecting))
    return false;

  bool result = false;

  struct addrinfo hints = {};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    goto exit;

  // Resolve the local address and port to be used by the server
  if (getaddrinfo(nullptr, port_.c_str(), &hints, &addrinfo_) != 0)
    goto exit;

  // Create a SOCKET for the server to listen for client connections
  listening_socket_ =
      socket(addrinfo_->ai_family, addrinfo_->ai_socktype, addrinfo_->ai_protocol);
  if (listening_socket_ == INVALID_SOCKET)
    goto exit;

  // Setup the TCP listening socket
  if (bind(listening_socket_, addrinfo_->ai_addr,
           static_cast<int>(addrinfo_->ai_addrlen)) == SOCKET_ERROR)
    goto exit;

  socket_event_ = WSACreateEvent();
  if (socket_event_ == WSA_INVALID_EVENT)
    goto exit;

  shutdown_event_ = CreateEvent(nullptr, false, false, nullptr);
  if (shutdown_event_ == nullptr)
    goto exit;

  if (WSAEventSelect(listening_socket_, socket_event_,
                     FD_ACCEPT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
    goto exit;

  if (listen(listening_socket_, SOMAXCONN) == SOCKET_ERROR)
    goto exit;

  socket_ = accept(listening_socket_, nullptr, nullptr);
  if (socket_ == INVALID_SOCKET) {
    if (WSAGetLastError() != WSAEWOULDBLOCK)
      goto exit;
  }

  {
    std::array<HANDLE, 2> wait_handles = { socket_event_, shutdown_event_ };
    DWORD wait_result = WaitForMultipleObjects(
        wait_handles.size(), &wait_handles.front(), false, INFINITE);
    if (wait_result == (WAIT_OBJECT_0 + 1) || wait_result >= WAIT_ABANDONED ||
        wait_result == WAIT_TIMEOUT || wait_result == WAIT_FAILED) {
      goto exit;
    }
  }

  socket_ = accept(socket_, nullptr, nullptr);
  if (socket_ == INVALID_SOCKET) {
    goto exit;
  }

  result = true;
  status_.store(ChannelStatus::Established);

exit:
  if (!result)
    Cleanup();

  return result;
}

void WinsockChannelImpl::Disconnect() {
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

bool WinsockChannelImpl::Read(void *buffer,
                              size_t buffer_size,
                              size_t *bytes_read) {
  // if (buffer == nullptr)
  //  *bytes_read = recv(socket_, buffer, buffer_size, 0);

  // WaitForMultipleObjects()

  //*bytes_read = recv(socket_, buffer, buffer_size, 0);
  return false;
}

bool WinsockChannelImpl::Write(void *buffer, size_t buffer_size) {
  return false;
}

}  // namespace nanorpc2
