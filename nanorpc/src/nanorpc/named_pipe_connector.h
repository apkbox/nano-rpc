#if !defined(NANO_RPC_NAMED_PIPE_CONNECTOR_HPP__)
#define NANO_RPC_NAMED_PIPE_CONNECTOR_HPP__

#include <string>
#include <mutex>
#include <condition_variable>

#include <windows.h>

#include "nanorpc/synchronization_primitives.hpp"

// The class facilitates named pipe connection sequence.
// If in server mode, once started, it listens for incoming
// connections and once client connected fires an event.
// The handler code could start listening for more connections if needed.
// If started in client mode, constantly polls for instance of pipe, and once
// pipe created, connects to it and fires an event.
// The opened pipe mode is compatible with NamedPipeRpcChannel implementation.
// The class does not own any opened pipe handles and it is client's
// responsibility to disconnect and free them.
namespace nanorpc {

class NamedPipeConnector {
public:
  enum ConnectionState { NotConnected, Connecting, Connected };

  class ICallback {
  public:
    virtual ~ICallback() {}
    virtual void Connected(HANDLE pipe) = 0;
  };

  NamedPipeConnector();
  NamedPipeConnector(const wchar_t *pipe_name,
                     const wchar_t *server_name = L".");

  std::wstring get_server_name() const { return server_name_; }
  void set_server_name(const wchar_t *server_name) {
    server_name_ = server_name == NULL ? L"." : server_name;
  }

  std::wstring get_pipe_name() const { return pipe_name_; }
  void set_pipe_name(const wchar_t *pipe_name) {
    pipe_name_ = pipe_name == NULL ? L"" : pipe_name;
  }

  void set_asynchronous(bool async) { is_asynchronous_connect_ = async; }

  bool is_asynchronous() const { return is_asynchronous_connect_; }

  void set_callback(ICallback *callback) { callback_ = callback; }
  ICallback *get_callback() { return callback_; }

  // Attempts to connect to the pipe.
  //
  // First it tries to do connection synchronously. If connection established,
  // then callback is called from the same thread that called StartConnection.
  // If synchronous attempt fails, the method schedules asynchronous connection
  // and returns immediately. If asynchronous connection succeeds, the callback
  // is called from pool thread and once callback returns, the thread
  // terminates.
  // This has a consequence that any overlapped operation started in the
  // callback would fail.
  // When callback called the pipe handle is passed as an argument. The callback
  // handler assumes ownership of the handle.
  //
  // If no connection is pending the method returns false if
  // fatal error occurred while trying make initial connection.
  //
  // If connection is already pending and specified connection side is same as
  // of the pending connection, then method returns true.
  bool StartConnection(bool client_side = false);

  void StopConnection();

  bool IsConnecting() const { return connection_state_ != Connecting; }

  ConnectionState get_connection_state() const {
    return static_cast<ConnectionState>(connection_state_);
  }

  HANDLE get_pipe() const { return pipe_; }

private:
  struct ConnectionContext {
    ConnectionContext(NamedPipeConnector *connector, HANDLE pipe)
        : connector_(connector), pipe_(pipe) {}

    NamedPipeConnector *connector_;
    HANDLE pipe_;
  };

  static DWORD CALLBACK WaitForClientThreadProcThunk(LPVOID parameter);
  static DWORD CALLBACK WaitForServerThreadProcThunk(LPVOID parameter);

  // Returns false if failed to create pipe instance.
  bool StartServerConnection();

  // Returns false if failed to setup polling for pipe connection.
  bool StartClientConnection();

  int WaitForClientThreadProc(HANDLE pipe);
  int WaitForServerThreadProc();

  // Attempts to accept client connection on the specified pipe instance
  // and if done synchronously, returns ERROR_SUCCESS.
  // Otherwise returns Windows error code.
  DWORD AttemptConnectToClient(HANDLE pipe);

  // Attempts to connect to the pipe instance.
  // Returns pipe handle if succeeded or INVALID_HANDLE_VALUE if failed.
  HANDLE AttemptConnectToServer();

  void InvokeConnectedCallback(HANDLE pipe);

  volatile __declspec(align(32)) LONG connection_state_;
  bool is_client_side_connection_;

  std::wstring server_name_;
  std::wstring pipe_name_;
  bool is_asynchronous_connect_;
  std::condition_variable cv_;
  std::mutex cv_mutex_;

  Event overlapped_event_;
  Event stop_connection_;
  OVERLAPPED overlapped_;

  ICallback *callback_;

  HANDLE pipe_;
};

}  // namespace

#endif  // NANO_RPC_NAMED_PIPE_CONNECTOR_HPP__
