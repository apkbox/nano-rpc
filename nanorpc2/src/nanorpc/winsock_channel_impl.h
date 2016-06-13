#if !defined(NANORPC_WINSOCK_CHANNEL_IMPL_H__)
#define NANORPC_WINSOCK_CHANNEL_IMPL_H__

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <winsock2.h>

#include "nanorpc/nanorpc2.h"
#include "nanorpc/winsock_channel.h"

namespace nanorpc2 {

class ScopedHandle {
public:
  ScopedHandle(HANDLE handle) : handle_(handle) {}
  ScopedHandle() : handle_(nullptr) {}
  ~ScopedHandle() {  }

  bool IsValid() const { return handle_ != nullptr; }

  HANDLE Get() const { return handle_; }
  operator HANDLE() const { return handle_; }

  HANDLE Take() {
    HANDLE tmp = handle_;
    handle_ = nullptr;
    return tmp;
  }

  void Set(HANDLE handle) {
    Close();
    handle_ = handle;
  }

  void Close() {
    if (handle_ != NULL)
      CloseHandle(handle_);
  }

private:
  HANDLE handle_;
};

class IoRequest final : public OVERLAPPED {
public:
  IoRequest() : pending_(false) {
    *static_cast<OVERLAPPED *>(this) = {};
  }

  ~IoRequest() {
    for (const auto buf : buffers_)
      delete buf.buf;
  }

  void set_pending(bool pending) { pending_ = pending; }
  bool get_pending() const { return pending_; }

  void *AllocateBuffer(size_t size) {
    WSABUF buf;
    buf.buf = new CHAR[size];
    buf.len = size;
    buffers_.push_back(buf);
    return reinterpret_cast<void *>(buf.buf);
  }

  LPWSABUF GetWSABUFPointer() {
    return &buffers_[0];
  }

  size_t GetWSABUFCount() const {
    return buffers_.size();
  }

private:
  std::vector<WSABUF> buffers_;
  bool pending_;  // Inidcates that I/O is in progress, so GetBuffer and other
                  // ops will fail.

  NANORPC_DISALLOW_COPY_AND_ASSIGN(IoRequest);
};

class WinsockChannelImpl {
public:
  explicit WinsockChannelImpl(const std::string &port);
  explicit WinsockChannelImpl(const std::string &address, const std::string &port);
  ~WinsockChannelImpl();

  ChannelStatus GetStatus() const;

  bool Connect();
  void Disconnect();

  IoRequest AllocateRequest();

  bool Read(void *buffer, size_t buffer_size, size_t *bytes_read);
  bool Write(const void *buffer, size_t buffer_size);

private:
  friend class IoRequest;

  bool ConnectServer();
  bool ConnectClient();

  void Cleanup();
  std::shared_ptr<IoRequest> CreateIoRequest();
  void DeleteIoRequest(IoRequest *io_request);

  void IoCompletionRoutine();

  std::string address_;
  std::string port_;
  int connect_timeout_;
  const bool is_client;

  ChannelStatus status_;

  struct addrinfo *addrinfo_;
  SOCKET listening_socket_;
  SOCKET socket_;

  std::mutex read_lock_;
  std::mutex write_lock_;
  ScopedHandle completion_port_;
  std::unique_ptr<std::thread> io_thread_;
  std::mutex io_requests_lock_;
  std::vector<std::shared_ptr<IoRequest>> io_requests_;

  NANORPC_DISALLOW_COPY_AND_ASSIGN(WinsockChannelImpl);
};

}  // namespace nanorpc2

#endif  // NANORPC_WINSOCK_CHANNEL_IMPL_H__
