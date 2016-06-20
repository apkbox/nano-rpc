#if !defined(NANORPC_WINSOCK_CHANNEL_IMPL_H__)
#define NANORPC_WINSOCK_CHANNEL_IMPL_H__

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <queue>
#include <string>
#include <thread>
#include <vector>

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

enum class IoType {
  None,
  Read,
  Write
};

class IoRequest final : public OVERLAPPED {
public:
  IoRequest() : io_type_(IoType::None) {
    *static_cast<OVERLAPPED *>(this) = {};
  }

  ~IoRequest() {
    for (const auto buf : buffers_)
      delete buf.buf;
  }

  void set_iotype(IoType io_type) { io_type_ = io_type; }
  IoType get_iotype() const { return io_type_; }

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
  IoType io_type_;  // If other than None - inidcates that I/O is in progress,
                    // so GetBuffer and other ops will fail.

  NANORPC_DISALLOW_COPY_AND_ASSIGN(IoRequest);
};

class IoPacket final {
public:
private:
};

class WinsockChannelImpl {
public:
  explicit WinsockChannelImpl(const std::string &port);
  explicit WinsockChannelImpl(const std::string &address, const std::string &port);
  ~WinsockChannelImpl();

  ChannelStatus GetStatus() const;

  bool Connect();
  void Shutdown();
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

  std::mutex io_requests_lock_;
  std::unordered_map<IoRequest *, std::shared_ptr<IoRequest>> io_requests_;

  NANORPC_DISALLOW_COPY_AND_ASSIGN(WinsockChannelImpl);
};

}  // namespace nanorpc2

#endif  // NANORPC_WINSOCK_CHANNEL_IMPL_H__
