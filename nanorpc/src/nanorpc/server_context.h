#if !defined(NANORPC_SERVER_CONTEXT_H__)
#define NANORPC_SERVER_CONTEXT_H__

#include <memory>
#include <string>

#include "nanorpc/nanorpc2.h"
#include "nanorpc/object_manager.h"
#include "nanorpc/service_manager.h"
#include "nanorpc/event_service.h"

namespace nanorpc {

class ServerChannelInterface;
class ServiceInterface;
class RpcCall;
class RpcMessage;

class ServerContext : public EventSourceInterface {
public:
  ServerContext(std::unique_ptr<ServerChannelInterface> &channel,
                std::shared_ptr<ServiceManager> &service_manager);
  ~ServerContext();

  // Wait for request and handle it.
  // This method explicitly serializes calls for all services.
  // This method can be called multiple times.
  // Calling from multiple threads results in waiting for request in
  // each thread. It is undetermined in which order calls will be satisfied.
  // The wait may be terminated by calling Shutdown.
  // If connection is not established, this method will call Connect on the
  // channel.
  // Returns true if request was satisfied, false if connection terminated or
  // Shutdown was called.
  bool WaitForSingleRequest();

  // Connects and waits for requests until connection is terminated or
  // Shutdown is called.
  // Attempt to call from another thread, while call in progress, will fail.
  // Returns true if connection terminated or Shutdown is called.
  // Returns false if method is called concurrently.
  bool ConnectAndWait();

  // Closes channel and shuts down the server.
  // This method can be called from any thread.
  void Shutdown();

  bool SendEvent(const RpcCall &call) override;

private:
  void ProcessRequest(const RpcMessage &request,
                      RpcMessage *response,
                      bool *is_async);

  std::unique_ptr<ServerChannelInterface> channel_;
  std::shared_ptr<ServiceManager> global_services_;
  ServiceManager context_services_;
  ObjectManager object_manager_;
  EventService event_service_;
};

}  // namespace nanorpc

#endif  // NANORPC_SERVER_CONTEXT_H__
