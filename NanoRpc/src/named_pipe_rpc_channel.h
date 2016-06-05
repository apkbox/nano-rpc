#if !defined(NANO_RPC_NAMED_PIPE_RPC_CHANNEL_HPP__)
#define NANO_RPC_NAMED_PIPE_RPC_CHANNEL_HPP__

#include <queue>
#include <mutex>

#include <windows.h>

#include "rpc_channel.h"
#include "buffer_pool.hpp"
#include "object_pool.hpp"
#include "callback.h"

namespace nanorpc2 {

class OverlappedOperation {
public:
  enum Type { Undefined, ReadPrefix, ReadMessage, Write };
};

class Overlapped : public OVERLAPPED {
public:
  class Initializer {
  public:
    void operator()(Overlapped *overlapped) { overlapped->Initialize(); }
  };

  Overlapped() { Initialize(); }

  ~Overlapped() {}

  void Initialize() {
    memset(static_cast<OVERLAPPED *>(this), 0, sizeof(OVERLAPPED));
    buffer = nullptr;
    operation_ = OverlappedOperation::Undefined;
  }

  char *buffer;
  OverlappedOperation::Type operation_;

private:
  Overlapped(const Overlapped &);
  Overlapped &operator=(const Overlapped &);
};

class NamedPipeRpcChannel;

class NamedPipeRpcChannelBuilder {
public:
  NamedPipeRpcChannelBuilder() : computer_(L"."), is_client_side_(false) {}

  NamedPipeRpcChannelBuilder(const wchar_t *name, const wchar_t *computer = L".") :
    pipe_name_(name), computer_(computer), is_client_side_(false) {
  
  }

  void set_pipe_name(const wchar_t *name) { pipe_name_ = name; }
  const std::wstring &get_pipe_name() const {
    return pipe_name_;
  }

  void set_computer_name(const wchar_t *name) { computer_ = name; }
  const std::wstring &get_computer_name() const {
    return computer_;
  }

  void set_connect_timeout(int timeout) {}
  int get_connect_timeout() const { return -1; }
  void set_reconnect(bool reconnect) { }
  bool get_reconnect() const { return false; }

  void set_client_side(bool is_client_side) {
    is_client_side_ = is_client_side;
  }
  bool get_client_side() const {
    return is_client_side_;
  }

  NamedPipeRpcChannel *Build();

private:
  std::wstring pipe_name_;
  std::wstring computer_;
  bool is_client_side_;
};


class NamedPipeRpcChannel : public RpcChannel {
public:
  ~NamedPipeRpcChannel();

  bool Connect() override;
  void Close() override;

  void Send(const void *message, size_t bytes) override;

  // Specifies a buffer and size of the buffer in bytes.
  // Returns true is message is available and number of bytes
  // copies to the message.
  // Returns false if there is no message and message and bytes remain unchanged.
  // message can be nullptr, in this case message size returned in bytes
  // if available.
  bool ReceiveNonBlocking(void *message, size_t *bytes) override;

  bool Receive(void *message, size_t *bytes) override;

  // Important: The handle passed in the disconnected callback is a closed pipe
  // handle.
  void set_disconnected_callback(CallbackBase<HANDLE> *callback) {
    disconnected_callback_ = callback;
  }
  CallbackBase<HANDLE> *get_disconnected_callback() {
    return disconnected_callback_;
  }

private:
  friend class NamedPipeRpcChannelBuilder;

  enum ChannelState { NotConnected = 0, Connected = 1, Disconnected = 2 };

  NamedPipeRpcChannel(const NamedPipeRpcChannelBuilder &builder);

  bool ConnectInternal(HANDLE pipe);

  int IoCompletionThreadProc();

  static DWORD WINAPI IoCompletionThreadProcThunk(void *parameter) {
    NamedPipeRpcChannel *channel =
        reinterpret_cast<NamedPipeRpcChannel *>(parameter);
    return channel->IoCompletionThreadProc();
  }

  void StartRead(int size, OverlappedOperation::Type requested_operation);

  void ReadOperationCompleted(Overlapped *overlapped, DWORD bytes_read);
  void WriteOperationCompleted(Overlapped *overlapped, DWORD bytes_written);

  HANDLE CloseConnection();

  void HandleSurpriseDisconnect();

  void InvokeDisconnectedCallback(HANDLE pipe);

  Overlapped *AllocateOverlappedState(int size);
  void FreeOverlappedState(Overlapped *overlapped);

  HANDLE pipe_;
  HANDLE completion_port_;
  DWORD completion_thread_id_;
  HANDLE completion_thread_;

  BufferPool buffer_pool_;
  ObjectPool<Overlapped, Overlapped::Initializer> overlapped_pool_;

  CallbackBase<HANDLE> *disconnected_callback_;

  struct InboundMessage {
    size_t bytes;
    Overlapped *message;
  };

  std::mutex queue_lock_;
  std::queue<InboundMessage> messages_;
  HANDLE message_pending_event_;

  volatile __declspec(align(32)) LONG channel_state_;  // TODO: Disconnected
                                                      // should be 0 = not
                                                      // connected, 1 =
                                                      // connected, 2 =
                                                      // disconnected/closed

  NamedPipeRpcChannelBuilder builder_;

  NamedPipeRpcChannel() = delete;
  NamedPipeRpcChannel(const NamedPipeRpcChannel&) = delete;
  NamedPipeRpcChannel &operator=(const NamedPipeRpcChannel&) = delete;
};

}  // namespace

#endif  // NANO_RPC_NAMED_PIPE_RPC_CHANNEL_HPP__
