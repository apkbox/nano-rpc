#if !defined(NANO_RPC_NAMED_PIPE_RPC_CHANNEL_HPP__)
#define NANO_RPC_NAMED_PIPE_RPC_CHANNEL_HPP__

#include <memory>
#include <mutex>
#include <queue>

#include <windows.h>

#include "rpc_channel.h"
#include "buffer_pool.hpp"
#include "object_pool.hpp"
#include "callback.h"

namespace nanorpc2 {

class NamedPipeChannel : public ChannelBuilder {
public:
  NamedPipeChannel() : computer_(L".") {}
  NamedPipeChannel(const wchar_t *name, const wchar_t *computer = L".");

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

  RpcChannel *BuildServerChannel();
  RpcChannel *BuildClientChannel();

private:
  std::wstring pipe_name_;
  std::wstring computer_;
};


}  // namespace

#endif  // NANO_RPC_NAMED_PIPE_RPC_CHANNEL_HPP__
