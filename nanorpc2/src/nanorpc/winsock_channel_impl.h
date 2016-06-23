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

class WinsockChannelImpl;

class ReadBufferImpl final : public ReadBuffer {
  friend class WinsockChannelImpl;
public:
  size_t GetSize() const override { return buffer_.size(); }
  size_t GetRemaining() const override { return ptr_ - &buffer_[0]; }

  const void *Peek(size_t bytes) const override {
    if (bytes > buffer_.size())
      return nullptr;
    return ptr_;
  }

  const void *Read(size_t bytes) override {
    if (((ptr_ - &buffer_[0]) + bytes) > buffer_.size())
      return nullptr;
    auto p = ptr_;
    ptr_ += bytes;
    return p;
  }

  virtual bool Skip(size_t bytes) override { return Read(bytes) != nullptr; }

  void Reset() override { ptr_ = &buffer_[0]; }

private:
  explicit ReadBufferImpl(size_t buffer_size)
      : buffer_(buffer_size), ptr_(&buffer_[0]) {}

  std::vector<unsigned char> buffer_;
  unsigned char *ptr_;
};

class WriteBufferImpl final : public WriteBuffer {
  friend class WinsockChannelImpl;
public:
  ~WriteBufferImpl();

  size_t GetSize() const override {
    if (current_->buffer == nullptr)
      return committed_;
    return committed_ + (ptr_ - current_->buffer);
  }

  void *Write(size_t bytes) override;

private:
  struct Chunk {
    unsigned char *buffer;
    size_t size;
    Chunk *next;
  };

  static const size_t kDefaultChunkSize = 512;

  explicit WriteBufferImpl() : current_(&head_), ptr_(nullptr), committed_(0) {
    head_.buffer = nullptr;
    head_.size = 0;
    head_.next = nullptr;
  }

  void AllocBufferIfNeeded(size_t bytes);

  void Flush();

  Chunk head_;
  Chunk *current_;
  unsigned char *ptr_;
  size_t committed_;
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

  std::unique_ptr<ReadBuffer> Read(size_t bytes);
  std::unique_ptr<WriteBuffer> CreateWriteBuffer();
  void Write(std::unique_ptr<WriteBuffer> buffer);

  bool Read(void *buffer, size_t buffer_size, size_t *bytes_read);
  bool Write(const void *buffer, size_t buffer_size);

private:
  bool ConnectServer();
  bool ConnectClient();

  void Cleanup();

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

  NANORPC_DISALLOW_COPY_AND_ASSIGN(WinsockChannelImpl);
};

}  // namespace nanorpc2

#endif  // NANORPC_WINSOCK_CHANNEL_IMPL_H__
