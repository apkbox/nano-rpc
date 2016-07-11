#include "nanorpc/winsock_buffers_impl.h"

#include <algorithm>

namespace nanorpc {

WriteBufferImpl::~WriteBufferImpl() {
  auto p = &head_;
  do {
    delete[] p->buffer;
    p = p->next;
  } while (p != nullptr);
}

void *WriteBufferImpl::Write(size_t bytes) {
  AllocBufferIfNeeded(bytes);

  // Check if current buffer is big enough to store the requested amount...
  auto avail = current_->size - (ptr_ - current_->buffer);
  if (avail < bytes) {
    // ...if not, store the amount of current buffer used
    current_->size = ptr_ - current_->buffer;
    committed_ += current_->size;

    // ...and create a new chunk big enough to store the requested amount.
    current_->next = new Chunk{};
    current_ = current_->next;
    AllocBufferIfNeeded(bytes);
  }

  auto p = ptr_;
  ptr_ += bytes;
  return p;
}

void WriteBufferImpl::AllocBufferIfNeeded(size_t bytes) {
  if (current_->buffer == nullptr) {
    current_->size = std::max(bytes, kDefaultChunkSize);
    current_->buffer = new unsigned char[current_->size];
    ptr_ = current_->buffer;
  }
}

void WriteBufferImpl::Flush() {
  if (current_->buffer == nullptr)
    return;

  current_->size = ptr_ - current_->buffer;
}

}  // namespace nanorpc
