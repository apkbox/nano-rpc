#if !defined(NANO_RPC_SYNCHRONIZATION_PRIMITIVES_HPP__)
#define NANO_RPC_SYNCHRONIZATION_PRIMITIVES_HPP__

#include <cassert>

#include <windows.h>

#include "nanorpc/basictypes.hpp"

namespace nanorpc {

class SynchronizationObject {
public:
  explicit SynchronizationObject(HANDLE handle) : handle_(handle) {}

  virtual ~SynchronizationObject() {
    if (handle_ != NULL)
      CloseHandle(handle_);
  }

  operator bool() const { return handle_ != NULL; }

  operator HANDLE() const { return handle_; }

  DWORD Wait(DWORD timeout = INFINITE) {
    assert(handle_ != NULL);
    return WaitForSingleObject(handle_, timeout);
  }

protected:
  SynchronizationObject() : handle_(NULL) {}

private:
  HANDLE handle_;

  DISALLOW_COPY_AND_ASSIGN(SynchronizationObject);
};

class Event : public SynchronizationObject {
public:
  Event();
  Event(bool manual, bool signalled);

  void Set();
  void Reset();

private:
  DISALLOW_COPY_AND_ASSIGN(Event);
};

class Semaphore : public SynchronizationObject {
public:
  Semaphore(int initial, int maximum);

  int Release(int count = 1);

private:
  DISALLOW_COPY_AND_ASSIGN(Semaphore);
};

class Lock {
public:
  Lock(unsigned int spin_count = 0);
  ~Lock();

  void Acquire();
  void Release();

private:
  CRITICAL_SECTION cs_;

  DISALLOW_COPY_AND_ASSIGN(Lock);
};

class ScopedLock {
public:
  ScopedLock(Lock &lock);
  ~ScopedLock();

private:
  Lock &lock_;

  DISALLOW_COPY_AND_ASSIGN(ScopedLock);
};

inline Lock::Lock(unsigned int spin_count) {
  InitializeCriticalSectionAndSpinCount(&cs_, spin_count);
}

inline Lock::~Lock() { DeleteCriticalSection(&cs_); }

inline void Lock::Acquire() { EnterCriticalSection(&cs_); }

inline void Lock::Release() { LeaveCriticalSection(&cs_); }

inline ScopedLock::ScopedLock(Lock &lock) : lock_(lock) { lock_.Acquire(); }

inline ScopedLock::~ScopedLock() { lock_.Release(); }

}  // namespace

#endif  // NANO_RPC_SYNCHRONIZATION_PRIMITIVES_HPP__
