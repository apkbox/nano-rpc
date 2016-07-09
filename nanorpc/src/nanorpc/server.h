#if !defined(NANORPC_SERVER_H__)
#define NANORPC_SERVER_H__

#include <memory>
#include <string>

#include "nanorpc/server_context.h"
#include "nanorpc/service_manager.h"

namespace nanorpc {

class ServerTransport;
class ServiceInterface;
class RpcCall;
class RpcMessage;

// TODO: Remove event source interface from base classes and provide
// method to get it.
class SimpleServer : public EventSourceInterface {
public:
  SimpleServer(std::unique_ptr<ServerTransport> transport);
  ~SimpleServer();

  void RegisterService(ServiceInterface *service);
  void RegisterService(const std::string &name, ServiceInterface *service);

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
  std::unique_ptr<ServerTransport> transport_;
  std::shared_ptr<ServiceManager> services_;
  std::unique_ptr<ServerContext> context_;

  NANORPC_DISALLOW_COPY_AND_ASSIGN(SimpleServer);
};

// TODO: Remove event source interface from base classes and provide
// method to get it.
class Server : public EventSourceInterface {
public:
  Server(std::unique_ptr<ServerTransport> transport);
  ~Server();

  void RegisterService(ServiceInterface *service);
  void RegisterService(const std::string &name, ServiceInterface *service);

  // Connects and waits for requests until connection is terminated or
  // Shutdown is called.
  // Attempt to call from another thread, while call in progress, will fail.
  // Returns true if connection terminated or Shutdown is called.
  // Returns false if method is called concurrently.
  bool Wait();

  // Closes channel and shuts down the server.
  // This method can be called from any thread.
  void Shutdown();

  bool SendEvent(const RpcCall &call) override;

private:
  class Context : public ServerContext {
  public:
    Context(std::unique_ptr<ChannelInterface> &channel,
      std::shared_ptr<ServiceManager> &service_manager);

    void Close();

  private:
    void ServerThread();
    std::thread thread_;
  };

  std::unique_ptr<ServerTransport> transport_;
  std::shared_ptr<ServiceManager> services_;
  std::vector<std::unique_ptr<Context>> contexts_;

  NANORPC_DISALLOW_COPY_AND_ASSIGN(Server);
};

}  // namespace nanorpc

#endif  // NANORPC_SERVER_H__
