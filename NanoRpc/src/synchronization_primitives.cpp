#include "synchronization_primitives.hpp"

#include <cassert>

#include <windows.h>

namespace NanoRpc {

Event::Event()
    : SynchronizationObject(::CreateEvent(NULL, TRUE, FALSE, NULL)) {}

Event::Event(bool manual, bool signalled)
    : SynchronizationObject(::CreateEvent(NULL, manual ? TRUE : FALSE,
                                          signalled ? TRUE : FALSE, NULL)) {}

void Event::Set() {
  assert(*this);
  SetEvent(*this);
}

void Event::Reset() {
  assert(*this);
  ResetEvent(*this);
}

Semaphore::Semaphore(int initial, int maximum)
    : SynchronizationObject(::CreateSemaphore(NULL, initial, maximum, NULL)) {
  assert(initial <= maximum);
  assert(*this);
}

int Semaphore::Release(int count) {
  assert(*this);

  LONG previuos_count;
  ReleaseSemaphore(*this, count, &previuos_count);
  return previuos_count;
}

} // namespace
