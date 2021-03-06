#include "named_pipe_connector.hpp"

#include <cassert>
#include <string>

#include <windows.h>

namespace NanoRpc {

NamedPipeConnector::NamedPipeConnector() : is_connecting_(0), callback_(NULL) {}

NamedPipeConnector::NamedPipeConnector(const wchar_t *pipe_name,
                                       const wchar_t *server_name)
    : is_connecting_(0), callback_(NULL) {
  set_server_name(server_name);
  set_pipe_name(pipe_name);
}

bool NamedPipeConnector::StartConnection(bool client_side) {
  if (InterlockedCompareExchange(&is_connecting_, 1, 0) == 0) {
    stop_connection_.Reset();
    is_client_side_connection_ = client_side;
    if (client_side)
      return StartClientConnection();
    else
      return StartServerConnection();
  } else {
    return client_side == is_client_side_connection_;
  }
}

void NamedPipeConnector::StopConnection() { stop_connection_.Set(); }

DWORD CALLBACK
NamedPipeConnector::WaitForClientThreadProcThunk(LPVOID parameter) {
  assert(parameter != NULL);

  ConnectionState *state = reinterpret_cast<ConnectionState *>(parameter);

  assert(state->connector_ != NULL);
  assert(state->pipe_ != INVALID_HANDLE_VALUE);

  NamedPipeConnector *connector = state->connector_;
  HANDLE pipe = state->pipe_;
  delete state;

  return connector->WaitForClientThreadProc(pipe);
}

DWORD CALLBACK
NamedPipeConnector::WaitForServerThreadProcThunk(LPVOID parameter) {
  assert(parameter != NULL);
  return reinterpret_cast<NamedPipeConnector *>(parameter)
      ->WaitForServerThreadProc();
}

bool NamedPipeConnector::StartServerConnection() {
  assert(!pipe_name_.empty());

  if (pipe_name_.empty()) {
    InterlockedExchange(&is_connecting_, 0);
    return false;
  }

  std::wstring full_pipe_name;
  full_pipe_name = L"\\\\.\\pipe\\" + pipe_name_;

  HANDLE pipe = CreateNamedPipe(
      full_pipe_name.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
      PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, PIPE_UNLIMITED_INSTANCES,
      1024,  // Output buffer size
      1024,  // Inpit buffer size
      1000,  // Default timeout = 1 second
      NULL); // Security attributes
  if (pipe == INVALID_HANDLE_VALUE) {
    InterlockedExchange(&is_connecting_, 0);
    return false;
  }

  DWORD err = AttemptConnectToClient(pipe);

  if (err == ERROR_SUCCESS) {
    return true;
  } else if (err == ERROR_IO_PENDING) {
    ConnectionState *state = new ConnectionState(this, pipe);
    if (QueueUserWorkItem(&WaitForClientThreadProcThunk, state,
                          WT_EXECUTELONGFUNCTION) == FALSE) {
      delete state;
      CancelIo(pipe);
      CloseHandle(pipe);
      InterlockedExchange(&is_connecting_, 0);
      return false;
    } else {
      // We do not change is_connecting_ here, because we are still pending the
      // connection
      // asynchronously in the other thread.
      return true;
    }
  } else {
    // Some other kind of error and operation have not even started.
    CloseHandle(pipe);
    InterlockedExchange(&is_connecting_, 0);
    return false;
  }
}

bool NamedPipeConnector::StartClientConnection() {
  assert(!server_name_.empty());
  assert(!pipe_name_.empty());

  if (server_name_.empty() || pipe_name_.empty()) {
    InterlockedExchange(&is_connecting_, 0);
    return false;
  }

  // First try to see if we can connect immediately and
  // avoid spanning a new thread.
  HANDLE pipe = AttemptConnectToServer();
  if (pipe != INVALID_HANDLE_VALUE) {
    InterlockedExchange(&is_connecting_, 0);
    InvokeConnectedCallback(pipe);
  } else {
    if (QueueUserWorkItem(&WaitForServerThreadProcThunk, this,
                          WT_EXECUTELONGFUNCTION) == FALSE) {
      InterlockedExchange(&is_connecting_, 0);
      return false;
    }
  }

  return true;
}

int NamedPipeConnector::WaitForClientThreadProc(HANDLE pipe) {
  assert(pipe != INVALID_HANDLE_VALUE);

  while (true) {
    // Wait until asynchronous operation completes
    HANDLE events[] = { overlapped_event_, stop_connection_ };
    DWORD result =
        WaitForMultipleObjects(arraysize(events), events, FALSE, INFINITE);
    if (result == (WAIT_OBJECT_0 + 1)) {
      InterlockedExchange(&is_connecting_, 0);
      break;
    }
    // WaitForSingleObject( overlapped_event_, INFINITE );   // TODO: should be
    // WaitForMultipleObjects to wait for cancellation too.

    // And get the result
    DWORD bytes_transferred;
    if (GetOverlappedResult(pipe, &overlapped_, &bytes_transferred, TRUE) ==
        FALSE) {
      if (GetLastError() == ERROR_IO_INCOMPLETE) {
        assert(false); // Theoretically should never get here.
                       // goto repeat_GetOverlappedResult;
      }
    } else {
      // Overlapped operation completed ok.
      InterlockedExchange(&is_connecting_, 0);
      InvokeConnectedCallback(pipe);
      break;
    }

    // Previous connection attempt failed, now try again.
    DWORD err = AttemptConnectToClient(pipe);
    if (err == ERROR_SUCCESS) {
      // Connected synchronously and callback is called for us.
      break;
    } else if (err != ERROR_IO_PENDING) {
      // If call fails immediately, there is possiblity that the thread would go
      // spinning.
      // So, we make sure that it does not go wild.
      Sleep(100);
    }
  }

  return 0;
}

int NamedPipeConnector::WaitForServerThreadProc() {
  while (true) {
    // TODO: Cancellation
    HANDLE pipe = AttemptConnectToServer();
    if (pipe != INVALID_HANDLE_VALUE) {
      InterlockedExchange(&is_connecting_, 0);
      InvokeConnectedCallback(pipe);
      break;
    }
  }

  return 0;
}

DWORD NamedPipeConnector::AttemptConnectToClient(HANDLE pipe) {
  assert(pipe != INVALID_HANDLE_VALUE);

  memset(&overlapped_, 0, sizeof(OVERLAPPED));
  overlapped_.hEvent = overlapped_event_;

  if (ConnectNamedPipe(pipe, &overlapped_) == FALSE) {
    DWORD err = GetLastError();
    if (err == ERROR_PIPE_CONNECTED) {
      // Connection completed synchronously.
      InterlockedExchange(&is_connecting_, 0);
      InvokeConnectedCallback(pipe);
      return ERROR_SUCCESS;
    } else {
      return err;
    }
  }

  // should not get here, because ConnectNamedPipe
  // completes either with ERROR_IO_PENDING, ERROR_PIPE_CONNECTED or
  // other error.
  // If we got here it is likely because the pipe was opened in non-overlapped
  // mode.
  assert(false);

  // Just for sake of sanity invoke the callback
  // (good if we later decide to support synchronous connect).
  InvokeConnectedCallback(pipe);

  return ERROR_SUCCESS;
}

HANDLE NamedPipeConnector::AttemptConnectToServer() {
  HANDLE pipe = INVALID_HANDLE_VALUE;

  std::wstring full_pipe_name;
  full_pipe_name = L"\\\\" + server_name_ + L"\\pipe\\" + pipe_name_;

  if (WaitNamedPipe(full_pipe_name.c_str(), NMPWAIT_USE_DEFAULT_WAIT) !=
      FALSE) {
    pipe = CreateFile(full_pipe_name.c_str(), GENERIC_READ | GENERIC_WRITE,
                      0,    // Sharing mode = none
                      NULL, // Security
                      OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL); // Template
  }

  return pipe;
}

void NamedPipeConnector::InvokeConnectedCallback(HANDLE pipe) {
  assert(callback_ != NULL);
  if (callback_ != NULL)
    callback_->Connected(pipe);
  else
    CloseHandle(pipe);
}

} // namespace
