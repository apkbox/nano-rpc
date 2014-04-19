#if !defined(NANO_RPC_OBJECT_POOL_HPP__)
#define NANO_RPC_OBJECT_POOL_HPP__

#include <cassert>
#include <hash_set>
#include <hash_map>

#include "basictypes.hpp"
#include "synchronization_primitives.hpp"

namespace NanoRpc {

// This is default object initializer.
template <class T>
class DefaultInitializer {
public:
  void operator()(T *object) {}
};

// Provides mechanism that allows to allocate and reuse objects.
// The object pool makes is faster to allocate often used objects
// and reduces memory fragmentation.
//
// This class is thread safe.
//
// When instantiating, the Initializer_ template argument may specify
// class that is used to initialize object when it is reused.
// The class should implement function call operator with the object
// type pointer argument.
template <class T, typename Initializer_ = DefaultInitializer<T>>
class ObjectPool {
  struct Descriptor {
    T *data;
    bool used;
  };

  typedef stdext::hash_map<const T *, Descriptor *> PointerToDescriptorMap;
  typedef stdext::hash_set<Descriptor *> DescriptorSet;

public:
  ObjectPool() {}

  ~ObjectPool() {
    ScopedLock lock(lock_);
    for (PointerToDescriptorMap::iterator iter = all_objects_.begin();
         iter != all_objects_.end();
         iter++) {
      Descriptor *descriptor = iter->second;
      assert(descriptor != NULL);
      assert(descriptor->data != NULL);
      assert(!descriptor->used);
      delete descriptor->data;
      descriptor->data = 0;
      delete descriptor;
    }
  }

  T *Allocate() {
    Descriptor *descriptor;

    ScopedLock lock(lock_);

    if (free_objects_.size() == 0) {
      descriptor = new Descriptor();
      descriptor->data = new T();
      all_objects_.insert(
          PointerToDescriptorMap::value_type(descriptor->data, descriptor));
    } else {
      DescriptorSet::iterator first = free_objects_.begin();
      descriptor = *first;
      free_objects_.erase(first);

      // TODO: Should we initialize it when returning to the pool instead?
      initializer_(descriptor->data);
    }

    descriptor->used = true;
    return descriptor->data;
  }

  void Deallocate(T *object) {
    assert(object != NULL);

    ScopedLock lock(lock_);

    // Check that object belongs to the pool
    PointerToDescriptorMap::const_iterator iter = all_objects_.find(object);
    assert(iter != all_objects_.end());
    if (iter == all_objects_.end())
      return;

    Descriptor *object_descriptor = iter->second;

    assert(object_descriptor->used);
    if (!object_descriptor->used)
      return;

#if !defined(NDEBUG)
    assert(free_objects_.find(object_descriptor) == free_objects_.end());
#endif

    object_descriptor->used = false;
    free_objects_.insert(object_descriptor);
  }

  size_t GetTotalObjectsCount() const { return all_objects_.size(); }

  size_t GetFreeObjectsCount() const { return free_objects_.size(); }

private:
  PointerToDescriptorMap all_objects_;
  DescriptorSet free_objects_;
  Initializer_ initializer_;

  Lock lock_;

  DISALLOW_COPY_AND_ASSIGN(ObjectPool);
};

}  // namespace

#endif  // NANO_RPC_OBJECT_POOL_HPP__
