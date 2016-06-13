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

#pragma comment(lib, "Ws2_32.lib")

namespace nanorpc2 {

WinsockChannelImpl::WinsockChannelImpl(const std::string &port)
    : address_(),
      port_(port),
      is_client(false),
      status_(ChannelStatus::NotConnected),
      addrinfo_(nullptr),
      listening_socket_(INVALID_SOCKET),
      socket_(INVALID_SOCKET) {}

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
    completion_port_ = CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket_), NULL, socket_, 0);
    io_thread_.reset(new std::thread(&WinsockChannelImpl::IoCompletionRoutine, this));

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
  listening_socket_ =
      socket(addrinfo_->ai_family, addrinfo_->ai_socktype, addrinfo_->ai_protocol);
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

  socket_ = accept(socket_, nullptr, nullptr);
  if (socket_ == INVALID_SOCKET) {
    goto exit;
  }

  result = true;
  status_ = ChannelStatus::Established;

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

  PostQueuedCompletionStatus(completion_port_, 0, 0, nullptr);
  if (io_thread_ != nullptr) {
    io_thread_->join();
    io_thread_.release();
  }

  completion_port_.Close();

  status_ = ChannelStatus::NotConnected;
}

std::shared_ptr<IoRequest> WinsockChannelImpl::CreateIoRequest() {
  std::shared_ptr<IoRequest> request(new IoRequest());
  std::lock_guard<std::mutex> lock(io_requests_lock_);
  io_requests_.push_back(request);
  return request;
}

void WinsockChannelImpl::DeleteIoRequest(IoRequest *request) {
  std::lock_guard<std::mutex> lock(io_requests_lock_);
  const auto iter = std::find_if(io_requests_.begin(), io_requests_.end(),
                           [request](const std::shared_ptr<IoRequest> &t) {
                             return request == t.get();
                           });
  if (iter == io_requests_.end())
    return;
  io_requests_.erase(iter);
}

void WinsockChannelImpl::IoCompletionRoutine() {
  bool wait_pending_io = false;

  while (true) {
    // For graceful shutdown we wait until all pending I/O completes.
    if (wait_pending_io) {
      std::lock_guard<std::mutex> lock(io_requests_lock_);
      if (io_requests_.size() == 0)
        break;
    }

    DWORD bytes_transferred;
    ULONG_PTR key;
    LPOVERLAPPED povl;
    if (GetQueuedCompletionStatus(completion_port_, &bytes_transferred, &key,
                                  &povl, INFINITE)) {
      if (key == 0 && povl == nullptr)
        wait_pending_io = true;

       auto io_request = reinterpret_cast<IoRequest *>(povl);
       DeleteIoRequest(io_request);
    }
  }
}

bool WinsockChannelImpl::Read(void *buffer,
                              size_t buffer_size,
                              size_t *bytes_read) {
  std::lock_guard<std::mutex> lock(read_lock_);

  // if (buffer == nullptr)
  //  *bytes_read = recv(socket_, buffer, buffer_size, 0);

  // WaitForMultipleObjects()

  //*bytes_read = recv(socket_, buffer, buffer_size, 0);
  return false;
}

bool WinsockChannelImpl::Write(const void *buffer, size_t buffer_size) {
  std::lock_guard<std::mutex> lock(write_lock_);

  auto request = CreateIoRequest();
  auto buf = request->AllocateBuffer(buffer_size);
  memcpy(buf, buffer, buffer_size);
  request->set_pending(true);
  int result =
      WSASend(socket_, request->GetWSABUFPointer(), request->GetWSABUFCount(),
              nullptr, 0, request.get(), nullptr);
  if (result == SOCKET_ERROR)
    return WSAGetLastError() == WSA_IO_PENDING;

  return true;
}

}  // namespace nanorpc2
