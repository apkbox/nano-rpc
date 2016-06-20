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
  std::shared_ptr<IoRequest> request(std::make_shared<IoRequest>());
  std::lock_guard<std::mutex> lock(io_requests_lock_);
  io_requests_.emplace(request.get(), request);
  return request;
}

void WinsockChannelImpl::DeleteIoRequest(IoRequest *request) {
  std::lock_guard<std::mutex> lock(io_requests_lock_);
  io_requests_.erase(request);
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
       if (io_request->get_iotype() == IoType::Read) {
         io_request->GetWSABUFPointer()->len = bytes_transferred;
         io_request->OffsetHigh = 0;
         io_request->Offset = 0;
         read_queue_.push(io_request);
         read_complete_cv_.notify_all();
       } else {
         DeleteIoRequest(io_request);
       }
    } else {
      OutputDebugString(L"GetQueueCompletionStatus: failed");
    }
  }
}

bool WinsockChannelImpl::Read(void *buffer,
                              size_t buffer_size,
                              size_t *bytes_read) {
  std::lock_guard<std::mutex> lock(read_lock_);

  // TODO: Algorithm
  //  - Check if there is already received data, if so, fill the buffer
  //  - Check if there is an outstanding read request, if not - issue one
  //  - Block if there was no received data

  // TODO: Do this in the loop and read data into the buffer until all data
  // read or buffer filled.
  // Then check if there are at least two outstanding read requests.
  if (!read_queue_.empty()) {
    const auto &iop = read_queue_.front();
    uint8_t *ptr = (uint8_t *)iop->GetWSABUFPointer()->buf;
    if (buffer == nullptr) {
      if (bytes_read == nullptr)
        return false;
      *bytes_read = iop->GetWSABUFPointer()->len - iop->Offset;
      return true;
    } else {
      size_t max_copy = std::min(buffer_size, (size_t)iop->GetWSABUFPointer()->len);
      memcpy(buffer, iop->GetWSABUFPointer()->buf, max_copy);
      iop->GetWSABUFPointer()->len -= max_copy;
      buffer_size -= max_copy;
      if (iop->GetWSABUFPointer()->len == 0)
        DeleteIoRequest(iop);

      if (bytes_read != nullptr)
        *bytes_read = max_copy;
      return true;
    }
  }

  auto request = CreateIoRequest();
  auto buf = request->AllocateBuffer(1000);
  request->set_iotype(IoType::Read);
  int result =
    WSARecv(socket_, request->GetWSABUFPointer(), request->GetWSABUFCount(),
    nullptr, 0, request.get(), nullptr);
  if (result == SOCKET_ERROR)
    return WSAGetLastError() == WSA_IO_PENDING;

  std::unique_lock<std::mutex> lk(read_complete_mtx_);
  read_complete_cv_.wait(lk, [this] { return !read_queue_.empty(); });

  return false;
}

bool WinsockChannelImpl::Write(const void *buffer, size_t buffer_size) {
  std::lock_guard<std::mutex> lock(write_lock_);

  auto request = CreateIoRequest();
  auto buf = request->AllocateBuffer(buffer_size);
  memcpy(buf, buffer, buffer_size);
  request->set_iotype(IoType::Write);
  int result =
      WSASend(socket_, request->GetWSABUFPointer(), request->GetWSABUFCount(),
              nullptr, 0, request.get(), nullptr);
  if (result == SOCKET_ERROR)
    return WSAGetLastError() == WSA_IO_PENDING;

  return true;
}

}  // namespace nanorpc2
