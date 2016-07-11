#if !defined(NANORPC_CLIENT_H__)
#define NANORPC_CLIENT_H__

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <deque>
#include <string>
#include <thread>
#include <unordered_map>

#include "nanorpc/nanorpc2.h"
#include "nanorpc/service_manager.h"

namespace nanorpc {

class ClientChannelInterface;
class ServiceInterface;
class RpcCall;
class RpcResult;
class RpcMessage;

// TODO: Big question is whether implicit connect is allowed.
// Should it be a config property (enabled by default) or not?
// Should client stick to RAII (so, if connection is lost, the only option
// is to create a new Client)?
class Client : public ServiceProxyInterface {
public:
  Client(std::unique_ptr<ClientChannelInterface> channel);
  ~Client();

  // Tells the server that the client is interested in events from
  // specific event interface.
  // The method creates a connection when client is not connected yet.
  // Returns true if subscription succeeded, false if server does not
  // support the event interface or connection failed.
  // The method does not create an event thread. The user should either
  // call CreateEventThread or call PumpEvents.
  // Note that at the moment the subscription interface is asynchronous,
  // so it always succeedes regardless whether the server supports an
  // event interface.
  bool StartListening(ServiceInterface *event_interface);
  bool StartListening(const std::string &name,
                      ServiceInterface *event_interface);

  // Stops listening for events on specified event interface.
  // The method does not stop event thread created via CreateEventThread.
  // All events in the event queue for this interface are discarded.
  void StopListening(const std::string &name);

  // Creates an event handling thread or thread pool.
  // concurrency specifies number of threads used to serve events concurrently.
  // Created thread(s) last until client is destroyed.
  void CreateEventThread(int concurrency = 0);

  // Handles a single event out of event queue.
  // Returns true if there are more events to pump.
  // If dropped returned true, then some events were dropped due to event
  // queue overflow since the last call of PumpEvents.
  // This method returns immediately if event queue is empty.
  // The dropped parameter can be nullptr.
  bool PumpEvents(bool *dropped);

  // Blocks until events are available in the event queue.
  // Returns true if events become available, false if connection is closed.
  bool WaitForEvents();

  // Connects and waits until connection is terminated, Shutdown or Disconnect
  // are called. Events will be serviced on the same thread that called
  // ConnectAndWait. Additional options can change the behavior to service
  // events with thread pool instead.
  // Attempt to call from another thread, while call in progress, will fail.
  // Returns true if connection terminated, Shutdown or Disconnect are called.
  // Returns false if method is called concurrently.
  bool ConnectAndWait();

  // Disconnects the client and closes the channel.
  // This method can be called from any thread.
  // The method blocks until all outstanding calls complete.
  // All calls made after this method is called will fail.
  // Incoming events will be ignored and those pending in the queue
  // will be discarded.
  void Shutdown();

  // Disconnects the client and closes the channel.
  // This method can be called from any thread.
  // All pending calls will fail.
  void Disconnect();

  // Calls RPC method and waits for reply.
  // TODO: If call is asynchronous the method should not block.
  // However, only proxy/stub know that the call is async, so
  // we should relay this information to CallMethod either within
  // RpcCall/RpcMessage (previous implementation) or via additional parameter.
  bool CallMethod(const RpcCall &rpc_call, RpcResult *rpc_result) override;

private:
  enum class PendingCallStatus { WaitingResult, Received, Cancelled };

  struct PendingCall {
    PendingCallStatus status;
    std::unique_ptr<RpcMessage> result;
  };

  struct EventCall {
    explicit EventCall(ServiceInterface *handler,
                       std::unique_ptr<RpcMessage> &call)
        : handler(handler), call(std::move(call)) {}

    EventCall(EventCall &&other)
        : handler(std::move(other.handler)), call(std::move(other.call)) {}

    EventCall& operator=(EventCall &&other) {
      handler = std::move(other.handler);
      call = std::move(other.call);
      return *this;
    }

    ServiceInterface *handler;
    std::unique_ptr<RpcMessage> call;
  };

  void SendCallRequest(uint32_t call_id, const RpcCall &rpc_call);
  bool WaitForResult(uint32_t call_id, RpcMessage *result);
  void ReceiveThreadProc();
  void LazyCreateReceiveThread();
  bool EnsureConnection();
  void FlushPendingCalls();
  void AbortPendingCalls();
  bool HandleIncomingMessage();

  std::unique_ptr<ClientChannelInterface> channel_;
  bool is_disposing = false;

  std::atomic<uint32_t> last_message_id_ = 0;
  std::mutex pending_calls_mtx_;
  std::condition_variable result_pending_cv_;
  std::unordered_map<pb::uint32, PendingCall> pending_calls_;

  ServiceManager event_service_manager_;
  std::mutex event_queue_mtx_;
  std::condition_variable event_pending_cv_;
  std::deque<EventCall> event_queue_;
  bool event_dropped_;

  std::once_flag receive_thread_started_;
  std::unique_ptr<std::thread> receive_thread_;
};

}  // namespace nanorpc

#endif  // NANORPC_CLIENT_H__
