#include "async_callback.hpp"

namespace NanoRpc {

void CALLBACK
AsyncCallbackBase::CallbackThunk(void *ptr, BOOLEAN timer_or_wait_fired) {
  AsyncCallbackBase *async = reinterpret_cast<AsyncCallbackBase *>(ptr);
  // If wait handle is registered with WT_EXECUTEDEFAULT, then UnregisterWait
  // causes recursion, because
  // canceling wait causes the callback to be invoked.
  UnregisterWait(async->wait_handle_);
  async->CallbackHandler(ptr, timer_or_wait_fired);
}

bool ThreadPool::RegisterAsyncIOOperation(HANDLE wait_handle,
                                          AsyncCallbackBase *callback) {
  return RegisterWaitForSingleObject(&callback->wait_handle_, wait_handle,
                                     *callback, callback, INFINITE,
                                     WT_EXECUTEONLYONCE) != FALSE;
}

} // namespace
