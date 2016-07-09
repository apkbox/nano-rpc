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

// TODO: Remove event source interface from base classes and provide
// method to get it.
class ServerContext : public EventSourceInterface {
public:
  ServerContext(std::unique_ptr<ChannelInterface> &channel,
                std::shared_ptr<ServiceManager> &service_manager);
  virtual ~ServerContext();

  // Wait for request and handle it.
  // This method explicitly serializes calls for all services.
  // This method can be called multiple times.
  // Calling from multiple threads results in waiting for request in
  // each thread. It is undetermined in which order calls will be satisfied.
  // The wait may be terminated by calling Close.
  // If connection is not established, this method will call Connect on the
  // channel.
  // Returns true if request was satisfied, false if connection terminated or
  // Shutdown was called.
  bool WaitForSingleRequest();

  // Closes channel and shuts down the server context.
  // This method can be called from any thread.
  void Close();

  bool SendEvent(const RpcCall &call) override;

private:
  void ProcessRequest(const RpcMessage &request,
                      RpcMessage *response,
                      bool *is_async);

  std::unique_ptr<ChannelInterface> channel_;
  std::shared_ptr<ServiceManager> global_services_;
  ServiceManager context_services_;
  ObjectManager object_manager_;
  EventService event_service_;

  NANORPC_DISALLOW_COPY_AND_ASSIGN(ServerContext);
};

}  // namespace nanorpc

#endif  // NANORPC_SERVER_CONTEXT_H__
