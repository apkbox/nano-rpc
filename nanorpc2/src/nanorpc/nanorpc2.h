#if !defined(NANORPC_NANORPC2_H__)
#define NANORPC_NANORPC2_H__

#include <map>
#include <memory>
#include <string>

#include "nanorpc/rpc_types.pb.h"

#define NANORPC_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName &) = delete;             \
  void operator=(const TypeName &) = delete

#define NANORPC_DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName() = delete;                                   \
  NANORPC_DISALLOW_COPY_AND_ASSIGN(TypeName)

namespace nanorpc2 {

using namespace nanorpc;
namespace pb = ::google::protobuf;

enum class ChannelStatus { NotConnected, Connecting, Established };

class ReadBuffer {
public:
  virtual ~ReadBuffer() {}

  // Returns total amount of data available in the buffer in bytes.
  virtual size_t GetSize() const = 0;

  // Returns amount of unread data in bytes.
  virtual size_t GetRemaining() const = 0;

  // Returns pointer to a section of the buffer at current position
  // that has at least 'bytes' number of bytes available in the stream.
  // Returns nullptr if requested number of bytes is not available.
  // This method does not advance read pointer.
  virtual const void *Peek(size_t bytes) const = 0;

  // Returns pointer to a section of the buffer at current position
  // that has at least 'bytes' number of bytes available in the stream
  // and advances the current read pointer.
  // Returns nullptr if requested number of bytes is not available.
  virtual const void *Read(size_t bytes) = 0;

  // Advances the read pointer for the specified number of bytes.
  // Returns true if requested number of bytes was skipped, false
  // if there is not enough unread bytes available in the buffer.
  virtual bool Skip(size_t bytes) = 0;

  // Resets read pointer back to the beginning.
  virtual void Reset() = 0;

  template <class T>
  bool PeekAs(T *t) const {
    if (t == nullptr)
      return false;
    auto p = reinterpret_cast<const T *>(Peek(sizeof(T)));
    if (p == nullptr)
      return false;
    *t = *p;
    return true;
  }

  template <class T>
  bool ReadAs(T *t) {
    if (t == nullptr)
      return false;
    auto p = reinterpret_cast<const T *>(Read(sizeof(T)));
    if (p == nullptr)
      return false;
    *t = *p;
    return true;
  }

  template <class T>
  const T *PeekAs() const {
    return reinterpret_cast<const T *>(Peek(sizeof(T)));
  }

  template <class T>
  const T *ReadAs() {
    return reinterpret_cast<const T *>(Read(sizeof(T)));
  }

  template <class T>
  bool SkipAs() {
    return Skip(sizeof(T));
  }
};

class WriteBuffer {
public:
  virtual ~WriteBuffer() {}

  // Returns number of bytes stored in the buffer.
  virtual size_t GetSize() const = 0;

  // Returns a pointer to a buffer segment big enough to store specified
  // number of bytes and advances the write pointer to the same amount.
  // The returned buffer segment is not initialized.
  virtual void *Write(size_t bytes) = 0;

  template <class T>
  T *WriteAs() {
    return reinterpret_cast<T *>(Write(sizeof(T)));
  }

  template <class T>
  void WriteAs(const T &t) {
    *reinterpret_cast<T *>(Write(sizeof(T))) = t;
  }
};

class StreamInterface {
public:
  virtual ~StreamInterface() {}

  virtual std::unique_ptr<ReadBuffer> Read(size_t bytes) = 0;
  virtual std::unique_ptr<WriteBuffer> CreateWriteBuffer() = 0;
  virtual void Write(std::unique_ptr<WriteBuffer> buffer) = 0;

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
  //virtual bool Read(void *buffer, size_t buffer_size, size_t *bytes_read) = 0;
  //virtual bool Write(void *buffer, size_t buffer_size) = 0;
};

class ChannelInterface : public StreamInterface {
public:
  virtual ~ChannelInterface() {}

  virtual ChannelStatus GetStatus() const = 0;

  virtual bool Connect() = 0;
  virtual void Shutdown() = 0;
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

class ServiceProxyInterface {
public:
  virtual ~ServiceProxyInterface() {}

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

private:
  void ProcessRequest(const RpcMessage &request,
                      RpcMessage *response,
                      bool *is_async);

  std::unique_ptr<ServerChannelInterface> channel_;
  ObjectManager object_manager_;
};

class Client : public ServiceProxyInterface {
public:
  Client(std::unique_ptr<ClientChannelInterface> channel);

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
  // All pending calls will fail.
  void Disconnect();

  bool CallMethod(const RpcCall &rpc_call, RpcResult *rpc_result) override;

private:
  struct PendingCall {};

  std::unique_ptr<ClientChannelInterface> channel_;

  uint32_t last_message_id_;
  std::map<pb::uint32, PendingCall> pending_calls_;
};

}  // namespace nanorpc2

#endif  // NANORPC_NANORPC2_H__
