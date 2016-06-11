#if !defined(NANORPC_NANORPC2_H__)
#define NANORPC_NANORPC2_H__

#include <string>
#include <map>
#include <memory>

#include "nanorpc/rpc_types.pb.h"

#define NANORPC_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName &) = delete;             \
  void operator=(const TypeName &) = delete

#define NANORPC_DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName() = delete;                                   \
  NANORPC_DISALLOW_COPY_AND_ASSIGN(TypeName)

namespace nanorpc2 {

using namespace nanorpc;

enum class ChannelStatus { NotConnected, Connecting, Established };

class StreamInterface {
public:
  virtual ~StreamInterface() {}

  // Reads data from stream. The function blocks until data is available.
  // If buffer is nullptr, returns true and number of available bytes
  // in byte_read. Returns false if channel is disconnected. buffer_size
  // parameter
  // is ignored in this case.
  // If buffer is not nullptr, then fills the buffer, number
  // of bytes read in bytes_read and returns true.
  // Returns false if buffer too small, channel disconnected
  // or connection terminated. If buffer too small, no data is read.
  // Note that the buffer must be sufficiently big to fill the whole message.
  virtual bool Read(void *buffer, size_t buffer_size, size_t *bytes_read) = 0;
  virtual bool Write(void *buffer, size_t buffer_size) = 0;
};

class ChannelInterface : public StreamInterface {
public:
  virtual ~ChannelInterface() {}

  virtual ChannelStatus GetStatus() const = 0;

  virtual bool Connect() = 0;
  virtual void Disconnect() = 0;
};

class ServerChannelInterface : public ChannelInterface {
public:
  virtual ~ServerChannelInterface() {}
};

class ClientChannelInterface : public ChannelInterface {
public:
  virtual ~ClientChannelInterface() {}
};

class ServiceInterface {
public:
  virtual ~ServiceInterface() {}

  virtual const std::string &GetInterfaceName() const = 0;

  // Returns true if call is synchronous, false otherwise.
  virtual bool CallMethod(const RpcCall &rpc_call, RpcResult *rpc_result) = 0;
};

typedef uint64_t RpcObjectId;

// TODO: This will only be needed when dealing with transient objects.
class ObjectManagerInterface {
public:
  virtual ~ObjectManagerInterface() {}
  virtual RpcObjectId AddObject(ServiceInterface *instance) = 0;
  virtual void DeleteObject(RpcObjectId object_id) = 0;
};

class ObjectManager : public ObjectManagerInterface {
public:
  ObjectManager();

  void AddService(const std::string &name, ServiceInterface *service);
  void RemoveService(const std::string &name);
  ServiceInterface *GetService(const std::string &name);

  // Currently implementation assumes that only unique instances are registered.
  // So, it is wrong if method returns the same object accross multiple calls.
  RpcObjectId AddObject(ServiceInterface *instance);
  void DeleteObject(RpcObjectId object_id);
  ServiceInterface *GetObject(RpcObjectId object_id);

private:
  ObjectManager(const ObjectManager &) = delete;
  ObjectManager &operator=(const ObjectManager &) = delete;

  RpcObjectId last_object_id_;

  std::map<std::string, ServiceInterface *> services_;
  std::map<RpcObjectId, std::unique_ptr<ServiceInterface>> objects_;
};

class Server {
public:
  Server(std::unique_ptr<ServerChannelInterface> channel);

  void RegisterService(ServiceInterface *service);
  void RegisterService(const std::string &name, ServiceInterface *service);

  // Wait for request and handle it.
  // This method explicitly serializes calls for all services.
  // This method can be called multiple times.
  // Calling from multiple threads results in waiting for request in
  // each thread. It is undetermined in which order calls will be satisfied.
  // The wait may be terminated by calling Shutdown.
  // Returns true if request was satisfied, false if connection terminated or
  // Shutdown is called.
  bool WaitForSingleRequest();

  // Connects and waits for requests until connection is terminated
  // Shutdown is called.
  // Attempt to call from another thread, while call in progress, will fail.
  // Returns true if connection terminated or Shutdown is called.
  // Returns false if method is called concurrently.
  bool ConnectAndWait();

  // Closes channel and shuts down the server.
  // This method can be called from any thread.
  void Shutdown();

private:
  void ProcessRequest(const RpcMessage &request,
                      RpcMessage *response,
                      bool *is_async);

  std::unique_ptr<ServerChannelInterface> channel_;
  ObjectManager object_manager_;
};

class Client {
public:
};

}  // namespace nanorpc2

#endif  // NANORPC_NANORPC2_H__
