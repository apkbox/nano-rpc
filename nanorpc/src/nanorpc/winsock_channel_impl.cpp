#include "nanorpc/winsock_channel_impl.h"

#include <algorithm>
#include <atomic>
#include <array>
#include <string>

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

#include "nanorpc/winsock_buffers_impl.h"

namespace nanorpc {

WinsockServerChannelImpl::WinsockServerChannelImpl(SOCKET socket)
    : status_(ChannelStatus::Established), socket_(socket) {
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
}

WinsockServerChannelImpl::~WinsockServerChannelImpl() {
  Disconnect();
}

void WinsockServerChannelImpl::Shutdown() {
  if (status_ == ChannelStatus::NotConnected)
    return;

  if (socket_ != INVALID_SOCKET) {
    int result = shutdown(socket_, SD_SEND);
    if (result == SOCKET_ERROR)
      Disconnect();
  }
}

void WinsockServerChannelImpl::Disconnect() {
  if (status_ == ChannelStatus::NotConnected)
    return;

  if (socket_ != INVALID_SOCKET) {
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
  }

  status_ = ChannelStatus::NotConnected;
}

std::unique_ptr<ReadBuffer> WinsockServerChannelImpl::Read(size_t bytes) {
  std::unique_ptr<ReadBufferImpl> buffer{new ReadBufferImpl(bytes)};
  size_t bytes_read;
  auto &buf = buffer.get()->buffer_;
  if (!Read(&buf[0], buf.size(), &bytes_read) || bytes_read == 0)
    return nullptr;
  return std::unique_ptr<ReadBuffer>(std::move(buffer));
}

std::unique_ptr<WriteBuffer> WinsockServerChannelImpl::CreateWriteBuffer() {
  return std::unique_ptr<WriteBuffer>{new WriteBufferImpl()};
}

void WinsockServerChannelImpl::Write(std::unique_ptr<WriteBuffer> buffer) {
  // TODO: This is not good
  WriteBufferImpl *raw_buffer = static_cast<WriteBufferImpl *>(buffer.get());
  raw_buffer->Flush();

  auto p = &raw_buffer->head_;
  if (p->buffer == nullptr)
    return;
  do {
    Write(p->buffer, p->size);
    p = p->next;
  } while (p != nullptr);
}

bool WinsockServerChannelImpl::Read(void *buffer,
                                    size_t buffer_size,
                                    size_t *bytes_read) {
  std::lock_guard<std::mutex> lock(read_lock_);

  *bytes_read = 0;

  char *ptr = reinterpret_cast<char *>(buffer);
  while (buffer_size > 0) {
    int result = recv(socket_, ptr, buffer_size, 0);
    if (result == SOCKET_ERROR)
      return WSAGetLastError() == WSA_IO_PENDING;

    if (result == 0) {
      *bytes_read = 0;
      return true;
    }

    if (result <= buffer_size) {
      ptr += result;
      buffer_size -= result;
      *bytes_read += result;
    }
  }

  return true;
}

bool WinsockServerChannelImpl::Write(const void *buffer, size_t buffer_size) {
  std::lock_guard<std::mutex> lock(write_lock_);

  int result =
      send(socket_, reinterpret_cast<const char *>(buffer), buffer_size, 0);
  if (result == SOCKET_ERROR)
    return WSAGetLastError() == WSA_IO_PENDING;

  return true;
}

void WinsockServerChannelImpl::Cleanup() {
  if (socket_ != INVALID_SOCKET) {
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
  }

  WSACleanup();
  status_ = ChannelStatus::NotConnected;
}

WinsockChannelImpl::WinsockChannelImpl(const std::string &address,
                                       const std::string &port)
    : address_(address),
      port_(port),
      is_client(true),
      status_(ChannelStatus::NotConnected),
      addrinfo_(nullptr),
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

  WSACleanup();
  status_ = ChannelStatus::NotConnected;
}

bool WinsockChannelImpl::ConnectClient() {
  if (status_ != ChannelStatus::NotConnected)
    return false;

  status_ = ChannelStatus::Connecting;

  bool failure = true;  // It is failure unless we explicitly said otherwise.

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

  bool connected = false;
  for (auto p = addrinfo_; p != nullptr; p = p->ai_next) {
    socket_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (socket_ == INVALID_SOCKET)
      continue;

    if (connect(socket_, p->ai_addr, static_cast<int>(p->ai_addrlen)) == 0) {
      connected = true;
      break;
    }

    closesocket(socket_);
    socket_ = INVALID_SOCKET;
  }

  if (connected) {
    failure = false;
    status_ = ChannelStatus::Established;
  }

exit:
  if (failure)
    Cleanup();

  return !failure;
}

bool WinsockChannelImpl::ConnectServer() {
  if (status_ != ChannelStatus::NotConnected)
    return false;

  status_ = ChannelStatus::Connecting;

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
  listening_socket_ = socket(addrinfo_->ai_family, addrinfo_->ai_socktype,
                             addrinfo_->ai_protocol);
  if (listening_socket_ == INVALID_SOCKET)
    goto exit;

  // Setup the TCP listening socket
  if (bind(listening_socket_, addrinfo_->ai_addr,
           static_cast<int>(addrinfo_->ai_addrlen)) == SOCKET_ERROR)
    goto exit;

  if (listen(listening_socket_, SOMAXCONN) == SOCKET_ERROR)
    goto exit;

  socket_ = accept(listening_socket_, nullptr, nullptr);
  if (socket_ == INVALID_SOCKET) {
    if (WSAGetLastError() != WSAEWOULDBLOCK)
      goto exit;
  }

  result = true;
  status_ = ChannelStatus::Established;

exit:
  if (!result)
    Cleanup();

  return result;
}

void WinsockChannelImpl::Shutdown() {
  if (status_ == ChannelStatus::NotConnected)
    return;

  if (socket_ != INVALID_SOCKET) {
    int result = shutdown(socket_, SD_SEND);
    if (result == SOCKET_ERROR)
      Disconnect();
  }
}

void WinsockChannelImpl::Disconnect() {
  if (status_ == ChannelStatus::NotConnected)
    return;

  if (socket_ != INVALID_SOCKET) {
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
  }

  status_ = ChannelStatus::NotConnected;
}

std::unique_ptr<ReadBuffer> WinsockChannelImpl::Read(size_t bytes) {
  std::unique_ptr<ReadBufferImpl> buffer{new ReadBufferImpl(bytes)};
  size_t bytes_read;
  auto &buf = buffer.get()->buffer_;
  if (!Read(&buf[0], buf.size(), &bytes_read) || bytes_read == 0)
    return nullptr;
  return std::unique_ptr<ReadBuffer>(std::move(buffer));
}

std::unique_ptr<WriteBuffer> WinsockChannelImpl::CreateWriteBuffer() {
  return std::unique_ptr<WriteBuffer>{new WriteBufferImpl()};
}

void WinsockChannelImpl::Write(std::unique_ptr<WriteBuffer> buffer) {
  // TODO: This is not good
  WriteBufferImpl *raw_buffer = static_cast<WriteBufferImpl *>(buffer.get());
  raw_buffer->Flush();

  auto p = &raw_buffer->head_;
  if (p->buffer == nullptr)
    return;
  do {
    Write(p->buffer, p->size);
    p = p->next;
  } while (p != nullptr);
}

bool WinsockChannelImpl::Read(void *buffer,
                              size_t buffer_size,
                              size_t *bytes_read) {
  std::lock_guard<std::mutex> lock(read_lock_);

  *bytes_read = 0;

  char *ptr = reinterpret_cast<char *>(buffer);
  while (buffer_size > 0) {
    int result = recv(socket_, ptr, buffer_size, 0);
    if (result == SOCKET_ERROR)
      return WSAGetLastError() == WSA_IO_PENDING;

    if (result == 0) {
      *bytes_read = 0;
      return true;
    }

    if (result <= buffer_size) {
      ptr += result;
      buffer_size -= result;
      *bytes_read += result;
    }
  }

  return true;
}

bool WinsockChannelImpl::Write(const void *buffer, size_t buffer_size) {
  std::lock_guard<std::mutex> lock(write_lock_);

  int result =
      send(socket_, reinterpret_cast<const char *>(buffer), buffer_size, 0);
  if (result == SOCKET_ERROR)
    return WSAGetLastError() == WSA_IO_PENDING;

  return true;
}

}  // namespace nanorpc
