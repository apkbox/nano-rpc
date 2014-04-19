#include "buffer_pool.hpp"

namespace NanoRpc {

namespace {

bool min_value_predicate(const std::pair<int, int> &a,
                         const std::pair<int, int> &b) {
  return a.second < b.second;
}

} // namespace

BufferPool::BufferPool(unsigned int pool_size) : pool_size_(pool_size) {}

BufferPool::~BufferPool() {
  ScopedLock lock(lock_);
  for (PointerToDescriptorMap::iterator iter = all_buffers_.begin();
       iter != all_buffers_.end(); iter++) {
    DestroyBufferInternal(iter->second);
  }
}

char *BufferPool::Allocate(int min_size) {
  assert(min_size > 0);

  Descriptor *descriptor = NULL;

  ScopedLock lock(lock_);

  SizeToDescriptorMap::iterator iter = free_buffers_.lower_bound(min_size);
  if (iter == free_buffers_.end()) {
    descriptor = CreateBuffer(min_size);
  } else {
    // Check if allocated buffer is too large. We want to prevent the situation
    // where occasional big buffer requests stuck in the pool, because they will
    // always satisfy smaller (or rather much smaller) requests.
    if (iter->first / 2 > min_size) {
      descriptor = CreateBuffer(min_size);
    } else {
      descriptor = iter->second;
      free_buffers_.erase(iter);
    }
  }

  descriptor->used = true;
  return descriptor->data;
}

void BufferPool::Deallocate(const char *buffer) {
  assert(buffer != NULL);

  ScopedLock lock(lock_);

  // Check that the buffer belongs to the pool.
  PointerToDescriptorMap::const_iterator iter = all_buffers_.find(buffer);
  assert(iter != all_buffers_.end());
  if (iter == all_buffers_.end())
    return;

  Descriptor *buffer_descriptor = iter->second;

  assert(buffer_descriptor->used);
  if (!buffer_descriptor->used)
    return;

#if !defined(NDEBUG)
  // This is failproof way to check if the buffer is already in the pool, but it
  // is slow.
  for (SizeToDescriptorMap::const_iterator iter = free_buffers_.begin();
       iter != free_buffers_.end(); iter++) {
    assert(iter->second != buffer_descriptor);
  }
#endif

  buffer_descriptor->used = false;

  use_statistics_[buffer_descriptor->size]++;

  // If there already enough free buffers, do not return the deallocated buffer
  // - just delete it.
  // We keep usage statistics which buffer sizes are used the most and those
  // which are kept in the pool.
  if (free_buffers_.size() >= pool_size_) {
    SizeToUsageMap::iterator iter = std::min_element(
        use_statistics_.begin(), use_statistics_.end(), min_value_predicate);
    assert(iter != use_statistics_.end());

    int buffer_size_to_erase = iter->first;
    if (buffer_descriptor->size == buffer_size_to_erase) {
      DestroyBuffer(buffer_descriptor);
      return;
    }

    // erase usage entry
    use_statistics_.erase(buffer_size_to_erase);

    // destroy all matching buffers
    for (SizeToDescriptorMap::iterator iter =
             free_buffers_.find(buffer_size_to_erase);
         iter != free_buffers_.end(); iter++) {
      if (iter->first != buffer_size_to_erase)
        break;
      DestroyBuffer(iter->second);
    }

    // ...and their entries
    free_buffers_.erase(buffer_size_to_erase);
  }

  free_buffers_.insert(free_buffers_.begin(),
                       SizeToDescriptorMap::value_type(buffer_descriptor->size,
                                                       buffer_descriptor));
}

int BufferPool::GetBufferSize(const char *buffer) const {
  assert(buffer != NULL);

  ScopedLock lock(lock_);

  // Check that buffer belongs to the pool
  PointerToDescriptorMap::const_iterator iter = all_buffers_.find(buffer);
  assert(iter != all_buffers_.end());
  if (iter == all_buffers_.end())
    throw std::exception("BufferPool::GetBufferSize: buffer does not belong to "
                         "this pool or buffer pointer is invalid.");

  // Check if someone attempts to fool us by providing buffer that is free but
  // still in the pool.
  assert(iter->second->used);
  if (!iter->second->used)
    throw std::exception("BufferPool::GetBufferSize: buffer is not allocated.");

  return iter->second->size;
}

BufferPool::Descriptor *BufferPool::CreateBuffer(int size) {
  assert(size > 0);

  Descriptor *descriptor = new Descriptor;

  try {
    descriptor->data = new char[size];
  }
  catch (std::bad_alloc &) {
    delete descriptor;
    throw;
  }
  descriptor->size = size;
  descriptor->used = true;
  all_buffers_.insert(
      PointerToDescriptorMap::value_type(descriptor->data, descriptor));
  return descriptor;
}

void BufferPool::DestroyBuffer(Descriptor *descriptor) {
  assert(descriptor != NULL);
  assert(descriptor->data != NULL);
  assert(!descriptor->used); // Attempt to delete buffer that is still in use

  all_buffers_.erase(descriptor->data);

  DestroyBufferInternal(descriptor);
}

void BufferPool::DestroyBufferInternal(Descriptor *descriptor) {
  assert(descriptor != NULL);
  assert(descriptor->data != NULL);
  assert(!descriptor->used); // Attempt to delete buffer that is still in use

  delete[] descriptor->data;
  descriptor->data = 0; // ensure assertion failure in attempt to use it
  delete descriptor;
}

} // namespace
