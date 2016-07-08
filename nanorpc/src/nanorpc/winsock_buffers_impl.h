#if !defined(NANORPC_WINSOCK_BUFFERS_IMPL_H_)
#define NANORPC_WINSOCK_BUFFERS_IMPL_H_

#include "nanorpc/winsock_channel.h"

namespace nanorpc {

class WinsockChannelImpl;

class ReadBufferImpl final : public ReadBuffer {
  friend class WinsockChannelImpl;
  friend class WinsockServerChannelImpl;
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
  friend class WinsockServerChannelImpl;
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

}  // namespace nanorpc

#endif  // NANORPC_WINSOCK_BUFFERS_IMPL_H_
