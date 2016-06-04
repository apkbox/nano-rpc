#if !defined(NANO_RPC_BUFFER_POOL_HPP__)
#define NANO_RPC_BUFFER_POOL_HPP__

#include <algorithm>
#include <cassert>
#include <map>

#include "basictypes.hpp"
#include "synchronization_primitives.hpp"

namespace nanorpc2 {

// Provides mechanism that allows to allocate and reuse buffers.
// The buffer pool makes is faster to allocate often used buffer sizes
// and reduces memory fragmentation in cases when many small buffers
// are used.
//
// This class is thread safe.
//
class BufferPool {
  struct Descriptor {
    char *data;
    int size;
    bool used;  // This only needed to optimize the lookup when deallocating the
                // buffer.
  };

  // There is a apparent bug in hash_map, so we use map instead, although
  // hash_map would be much better.
  // Note that almost the same code works in object_pool no problem.
  typedef std::map<const char *, Descriptor *> PointerToDescriptorMap;
  typedef std::multimap<int, Descriptor *> SizeToDescriptorMap;
  typedef std::map<int, int> SizeToUsageMap;  // size,usage
  static const unsigned int DefaultPoolSize =
      32;  // Maximum number of free buffers in the pool

public:
  BufferPool(unsigned int pool_size = DefaultPoolSize);
  ~BufferPool();

  size_t GetTotalBuffersCount() const { return all_buffers_.size(); }

  size_t GetFreeBuffersCount() const { return free_buffers_.size(); }

  // Gets maximum number of buffers that pool may have.
  unsigned int get_pool_size() const { return pool_size_; }

  // Sets maximum number of buffers that pool may have. If number of
  // buffers exceed the specified value, then excess of least used
  // buffers deleted from the pool.
  void set_pool_size(unsigned int pool_size) { pool_size_ = pool_size; }

  // Allocates a new buffer of the specified size. The actual buffer size may be
  // bigger.
  // If allocation failed, std::bad_alloc exception is thrown.
  char *Allocate(int min_size);

  // Deallocates the buffer. The buffer may be returned in the pool.
  // After the call the buffer pointer is not valid.
  // If the buffer does not belong to the pool or an invalid pointer is passed,
  // then
  // exception is thrown.
  void Deallocate(const char *buffer);

  // Gets the size of previously allocated buffer.
  // If the buffer does not belong to the pool or an invalid pointer is passed,
  // then
  // exception is thrown.
  int GetBufferSize(const char *buffer) const;

private:
  Descriptor *CreateBuffer(int size);
  void DestroyBuffer(Descriptor *buffer);
  static void DestroyBufferInternal(Descriptor *buffer);

  unsigned int pool_size_;
  PointerToDescriptorMap all_buffers_;
  SizeToDescriptorMap free_buffers_;
  SizeToUsageMap use_statistics_;

  mutable Lock lock_;

  DISALLOW_COPY_AND_ASSIGN(BufferPool);
};

}  // namespace

#endif  // NANO_RPC_BUFFER_POOL_HPP__
