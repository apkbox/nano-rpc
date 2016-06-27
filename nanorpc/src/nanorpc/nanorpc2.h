#if !defined(NANORPC_NANORPC2_H__)
#define NANORPC_NANORPC2_H__

#include <memory>
#include <string>

#include "nanorpc/rpc_types.pb.h"

#define NANORPC_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName &) = delete;             \
  void operator=(const TypeName &) = delete

#define NANORPC_DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName() = delete;                                   \
  NANORPC_DISALLOW_COPY_AND_ASSIGN(TypeName)

namespace nanorpc {

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
  T *WriteAsArray(size_t ncount) {
    return reinterpret_cast<T *>(Write(sizeof(T) * ncount));
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

class CallHandlerInterface {
public:
  virtual ~CallHandlerInterface() {}
  virtual bool CallMethod(const RpcCall &rpc_call, RpcResult *rpc_result) = 0;
};

class EventSourceInterface {
public:
  virtual ~EventSourceInterface() {}
  virtual bool SendEvent(const RpcCall &call) = 0;
};

class ServiceInterface : public CallHandlerInterface {
public:
  virtual ~ServiceInterface() {}
  virtual const std::string &GetInterfaceName() const = 0;
  // CallMethod returns true if call is synchronous, false otherwise.
};

class ServiceProxyInterface : public CallHandlerInterface {
public:
  virtual ~ServiceProxyInterface() {}
};

typedef uint64_t RpcObjectId;

// TODO: This will only be needed when dealing with transient objects.
class ObjectManagerInterface {
public:
  virtual ~ObjectManagerInterface() {}
  virtual RpcObjectId AddObject(ServiceInterface *instance) = 0;
  virtual void DeleteObject(RpcObjectId object_id) = 0;
};


}  // namespace nanorpc

#endif  // NANORPC_NANORPC2_H__
