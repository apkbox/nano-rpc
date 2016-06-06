// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: rpc_proto/rpc_types.proto

#ifndef PROTOBUF_rpc_5fproto_2frpc_5ftypes_2eproto__INCLUDED
#define PROTOBUF_rpc_5fproto_2frpc_5ftypes_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
#include <google/protobuf/descriptor.pb.h>
// @@protoc_insertion_point(includes)

namespace nanorpc2 {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_rpc_5fproto_2frpc_5ftypes_2eproto();
void protobuf_AssignDesc_rpc_5fproto_2frpc_5ftypes_2eproto();
void protobuf_ShutdownFile_rpc_5fproto_2frpc_5ftypes_2eproto();

class RpcCall;
class RpcEvent;
class RpcMessage;
class RpcObject;
class RpcResult;
class RpcVoid;

enum RpcStatus {
  RpcSucceeded = 0,
  RpcChannelFailure = 1,
  RpcUnknownMethod = 2,
  RpcProtocolError = 3,
  RpcUnknownInterface = 4,
  RpcInvalidCallParameter = 5,
  RpcStatus_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  RpcStatus_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool RpcStatus_IsValid(int value);
const RpcStatus RpcStatus_MIN = RpcSucceeded;
const RpcStatus RpcStatus_MAX = RpcInvalidCallParameter;
const int RpcStatus_ARRAYSIZE = RpcStatus_MAX + 1;

const ::google::protobuf::EnumDescriptor* RpcStatus_descriptor();
inline const ::std::string& RpcStatus_Name(RpcStatus value) {
  return ::google::protobuf::internal::NameOfEnum(
    RpcStatus_descriptor(), value);
}
inline bool RpcStatus_Parse(
    const ::std::string& name, RpcStatus* value) {
  return ::google::protobuf::internal::ParseNamedEnum<RpcStatus>(
    RpcStatus_descriptor(), name, value);
}
// ===================================================================

class RpcCall : public ::google::protobuf::Message {
 public:
  RpcCall();
  virtual ~RpcCall();

  RpcCall(const RpcCall& from);

  inline RpcCall& operator=(const RpcCall& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const RpcCall& default_instance();

  void Swap(RpcCall* other);

  // implements Message ----------------------------------------------

  inline RpcCall* New() const { return New(NULL); }

  RpcCall* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const RpcCall& from);
  void MergeFrom(const RpcCall& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(RpcCall* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional string service = 1;
  void clear_service();
  static const int kServiceFieldNumber = 1;
  const ::std::string& service() const;
  void set_service(const ::std::string& value);
  void set_service(const char* value);
  void set_service(const char* value, size_t size);
  ::std::string* mutable_service();
  ::std::string* release_service();
  void set_allocated_service(::std::string* service);

  // optional string method = 2;
  void clear_method();
  static const int kMethodFieldNumber = 2;
  const ::std::string& method() const;
  void set_method(const ::std::string& value);
  void set_method(const char* value);
  void set_method(const char* value, size_t size);
  ::std::string* mutable_method();
  ::std::string* release_method();
  void set_allocated_method(::std::string* method);

  // optional uint32 object_id = 3;
  void clear_object_id();
  static const int kObjectIdFieldNumber = 3;
  ::google::protobuf::uint32 object_id() const;
  void set_object_id(::google::protobuf::uint32 value);

  // optional bytes call_data = 4;
  void clear_call_data();
  static const int kCallDataFieldNumber = 4;
  const ::std::string& call_data() const;
  void set_call_data(const ::std::string& value);
  void set_call_data(const char* value);
  void set_call_data(const void* value, size_t size);
  ::std::string* mutable_call_data();
  ::std::string* release_call_data();
  void set_allocated_call_data(::std::string* call_data);

  // @@protoc_insertion_point(class_scope:nanorpc2.RpcCall)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr service_;
  ::google::protobuf::internal::ArenaStringPtr method_;
  ::google::protobuf::internal::ArenaStringPtr call_data_;
  ::google::protobuf::uint32 object_id_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_rpc_5fproto_2frpc_5ftypes_2eproto();
  friend void protobuf_AssignDesc_rpc_5fproto_2frpc_5ftypes_2eproto();
  friend void protobuf_ShutdownFile_rpc_5fproto_2frpc_5ftypes_2eproto();

  void InitAsDefaultInstance();
  static RpcCall* default_instance_;
};
// -------------------------------------------------------------------

class RpcResult : public ::google::protobuf::Message {
 public:
  RpcResult();
  virtual ~RpcResult();

  RpcResult(const RpcResult& from);

  inline RpcResult& operator=(const RpcResult& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const RpcResult& default_instance();

  void Swap(RpcResult* other);

  // implements Message ----------------------------------------------

  inline RpcResult* New() const { return New(NULL); }

  RpcResult* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const RpcResult& from);
  void MergeFrom(const RpcResult& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(RpcResult* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional .nanorpc2.RpcStatus status = 1;
  void clear_status();
  static const int kStatusFieldNumber = 1;
  ::nanorpc2::RpcStatus status() const;
  void set_status(::nanorpc2::RpcStatus value);

  // optional string error_message = 2;
  void clear_error_message();
  static const int kErrorMessageFieldNumber = 2;
  const ::std::string& error_message() const;
  void set_error_message(const ::std::string& value);
  void set_error_message(const char* value);
  void set_error_message(const char* value, size_t size);
  ::std::string* mutable_error_message();
  ::std::string* release_error_message();
  void set_allocated_error_message(::std::string* error_message);

  // optional bytes result_data = 3;
  void clear_result_data();
  static const int kResultDataFieldNumber = 3;
  const ::std::string& result_data() const;
  void set_result_data(const ::std::string& value);
  void set_result_data(const char* value);
  void set_result_data(const void* value, size_t size);
  ::std::string* mutable_result_data();
  ::std::string* release_result_data();
  void set_allocated_result_data(::std::string* result_data);

  // @@protoc_insertion_point(class_scope:nanorpc2.RpcResult)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr error_message_;
  ::google::protobuf::internal::ArenaStringPtr result_data_;
  int status_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_rpc_5fproto_2frpc_5ftypes_2eproto();
  friend void protobuf_AssignDesc_rpc_5fproto_2frpc_5ftypes_2eproto();
  friend void protobuf_ShutdownFile_rpc_5fproto_2frpc_5ftypes_2eproto();

  void InitAsDefaultInstance();
  static RpcResult* default_instance_;
};
// -------------------------------------------------------------------

class RpcMessage : public ::google::protobuf::Message {
 public:
  RpcMessage();
  virtual ~RpcMessage();

  RpcMessage(const RpcMessage& from);

  inline RpcMessage& operator=(const RpcMessage& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const RpcMessage& default_instance();

  void Swap(RpcMessage* other);

  // implements Message ----------------------------------------------

  inline RpcMessage* New() const { return New(NULL); }

  RpcMessage* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const RpcMessage& from);
  void MergeFrom(const RpcMessage& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(RpcMessage* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 id = 1;
  void clear_id();
  static const int kIdFieldNumber = 1;
  ::google::protobuf::int32 id() const;
  void set_id(::google::protobuf::int32 value);

  // optional .nanorpc2.RpcCall call = 2;
  bool has_call() const;
  void clear_call();
  static const int kCallFieldNumber = 2;
  const ::nanorpc2::RpcCall& call() const;
  ::nanorpc2::RpcCall* mutable_call();
  ::nanorpc2::RpcCall* release_call();
  void set_allocated_call(::nanorpc2::RpcCall* call);

  // optional .nanorpc2.RpcResult result = 3;
  bool has_result() const;
  void clear_result();
  static const int kResultFieldNumber = 3;
  const ::nanorpc2::RpcResult& result() const;
  ::nanorpc2::RpcResult* mutable_result();
  ::nanorpc2::RpcResult* release_result();
  void set_allocated_result(::nanorpc2::RpcResult* result);

  // @@protoc_insertion_point(class_scope:nanorpc2.RpcMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::nanorpc2::RpcCall* call_;
  ::nanorpc2::RpcResult* result_;
  ::google::protobuf::int32 id_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_rpc_5fproto_2frpc_5ftypes_2eproto();
  friend void protobuf_AssignDesc_rpc_5fproto_2frpc_5ftypes_2eproto();
  friend void protobuf_ShutdownFile_rpc_5fproto_2frpc_5ftypes_2eproto();

  void InitAsDefaultInstance();
  static RpcMessage* default_instance_;
};
// -------------------------------------------------------------------

class RpcVoid : public ::google::protobuf::Message {
 public:
  RpcVoid();
  virtual ~RpcVoid();

  RpcVoid(const RpcVoid& from);

  inline RpcVoid& operator=(const RpcVoid& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const RpcVoid& default_instance();

  void Swap(RpcVoid* other);

  // implements Message ----------------------------------------------

  inline RpcVoid* New() const { return New(NULL); }

  RpcVoid* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const RpcVoid& from);
  void MergeFrom(const RpcVoid& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(RpcVoid* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // @@protoc_insertion_point(class_scope:nanorpc2.RpcVoid)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_rpc_5fproto_2frpc_5ftypes_2eproto();
  friend void protobuf_AssignDesc_rpc_5fproto_2frpc_5ftypes_2eproto();
  friend void protobuf_ShutdownFile_rpc_5fproto_2frpc_5ftypes_2eproto();

  void InitAsDefaultInstance();
  static RpcVoid* default_instance_;
};
// -------------------------------------------------------------------

class RpcObject : public ::google::protobuf::Message {
 public:
  RpcObject();
  virtual ~RpcObject();

  RpcObject(const RpcObject& from);

  inline RpcObject& operator=(const RpcObject& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const RpcObject& default_instance();

  void Swap(RpcObject* other);

  // implements Message ----------------------------------------------

  inline RpcObject* New() const { return New(NULL); }

  RpcObject* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const RpcObject& from);
  void MergeFrom(const RpcObject& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(RpcObject* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional uint32 object_id = 1;
  void clear_object_id();
  static const int kObjectIdFieldNumber = 1;
  ::google::protobuf::uint32 object_id() const;
  void set_object_id(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:nanorpc2.RpcObject)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::uint32 object_id_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_rpc_5fproto_2frpc_5ftypes_2eproto();
  friend void protobuf_AssignDesc_rpc_5fproto_2frpc_5ftypes_2eproto();
  friend void protobuf_ShutdownFile_rpc_5fproto_2frpc_5ftypes_2eproto();

  void InitAsDefaultInstance();
  static RpcObject* default_instance_;
};
// -------------------------------------------------------------------

class RpcEvent : public ::google::protobuf::Message {
 public:
  RpcEvent();
  virtual ~RpcEvent();

  RpcEvent(const RpcEvent& from);

  inline RpcEvent& operator=(const RpcEvent& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const RpcEvent& default_instance();

  void Swap(RpcEvent* other);

  // implements Message ----------------------------------------------

  inline RpcEvent* New() const { return New(NULL); }

  RpcEvent* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const RpcEvent& from);
  void MergeFrom(const RpcEvent& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(RpcEvent* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional string event_name = 1;
  void clear_event_name();
  static const int kEventNameFieldNumber = 1;
  const ::std::string& event_name() const;
  void set_event_name(const ::std::string& value);
  void set_event_name(const char* value);
  void set_event_name(const char* value, size_t size);
  ::std::string* mutable_event_name();
  ::std::string* release_event_name();
  void set_allocated_event_name(::std::string* event_name);

  // @@protoc_insertion_point(class_scope:nanorpc2.RpcEvent)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr event_name_;
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_rpc_5fproto_2frpc_5ftypes_2eproto();
  friend void protobuf_AssignDesc_rpc_5fproto_2frpc_5ftypes_2eproto();
  friend void protobuf_ShutdownFile_rpc_5fproto_2frpc_5ftypes_2eproto();

  void InitAsDefaultInstance();
  static RpcEvent* default_instance_;
};
// ===================================================================

static const int kEventSourceFieldNumber = 51224;
extern ::google::protobuf::internal::ExtensionIdentifier< ::google::protobuf::MessageOptions,
    ::google::protobuf::internal::PrimitiveTypeTraits< bool >, 8, false >
  event_source;
static const int kPropertyNameFieldNumber = 51223;
extern ::google::protobuf::internal::ExtensionIdentifier< ::google::protobuf::MethodOptions,
    ::google::protobuf::internal::StringTypeTraits, 9, false >
  property_name;
static const int kAsyncFieldNumber = 51225;
extern ::google::protobuf::internal::ExtensionIdentifier< ::google::protobuf::MethodOptions,
    ::google::protobuf::internal::PrimitiveTypeTraits< bool >, 8, false >
  async;

// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// RpcCall

// optional string service = 1;
inline void RpcCall::clear_service() {
  service_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& RpcCall::service() const {
  // @@protoc_insertion_point(field_get:nanorpc2.RpcCall.service)
  return service_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcCall::set_service(const ::std::string& value) {
  
  service_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:nanorpc2.RpcCall.service)
}
inline void RpcCall::set_service(const char* value) {
  
  service_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:nanorpc2.RpcCall.service)
}
inline void RpcCall::set_service(const char* value, size_t size) {
  
  service_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:nanorpc2.RpcCall.service)
}
inline ::std::string* RpcCall::mutable_service() {
  
  // @@protoc_insertion_point(field_mutable:nanorpc2.RpcCall.service)
  return service_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcCall::release_service() {
  // @@protoc_insertion_point(field_release:nanorpc2.RpcCall.service)
  
  return service_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcCall::set_allocated_service(::std::string* service) {
  if (service != NULL) {
    
  } else {
    
  }
  service_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), service);
  // @@protoc_insertion_point(field_set_allocated:nanorpc2.RpcCall.service)
}

// optional string method = 2;
inline void RpcCall::clear_method() {
  method_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& RpcCall::method() const {
  // @@protoc_insertion_point(field_get:nanorpc2.RpcCall.method)
  return method_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcCall::set_method(const ::std::string& value) {
  
  method_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:nanorpc2.RpcCall.method)
}
inline void RpcCall::set_method(const char* value) {
  
  method_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:nanorpc2.RpcCall.method)
}
inline void RpcCall::set_method(const char* value, size_t size) {
  
  method_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:nanorpc2.RpcCall.method)
}
inline ::std::string* RpcCall::mutable_method() {
  
  // @@protoc_insertion_point(field_mutable:nanorpc2.RpcCall.method)
  return method_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcCall::release_method() {
  // @@protoc_insertion_point(field_release:nanorpc2.RpcCall.method)
  
  return method_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcCall::set_allocated_method(::std::string* method) {
  if (method != NULL) {
    
  } else {
    
  }
  method_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), method);
  // @@protoc_insertion_point(field_set_allocated:nanorpc2.RpcCall.method)
}

// optional uint32 object_id = 3;
inline void RpcCall::clear_object_id() {
  object_id_ = 0u;
}
inline ::google::protobuf::uint32 RpcCall::object_id() const {
  // @@protoc_insertion_point(field_get:nanorpc2.RpcCall.object_id)
  return object_id_;
}
inline void RpcCall::set_object_id(::google::protobuf::uint32 value) {
  
  object_id_ = value;
  // @@protoc_insertion_point(field_set:nanorpc2.RpcCall.object_id)
}

// optional bytes call_data = 4;
inline void RpcCall::clear_call_data() {
  call_data_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& RpcCall::call_data() const {
  // @@protoc_insertion_point(field_get:nanorpc2.RpcCall.call_data)
  return call_data_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcCall::set_call_data(const ::std::string& value) {
  
  call_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:nanorpc2.RpcCall.call_data)
}
inline void RpcCall::set_call_data(const char* value) {
  
  call_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:nanorpc2.RpcCall.call_data)
}
inline void RpcCall::set_call_data(const void* value, size_t size) {
  
  call_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:nanorpc2.RpcCall.call_data)
}
inline ::std::string* RpcCall::mutable_call_data() {
  
  // @@protoc_insertion_point(field_mutable:nanorpc2.RpcCall.call_data)
  return call_data_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcCall::release_call_data() {
  // @@protoc_insertion_point(field_release:nanorpc2.RpcCall.call_data)
  
  return call_data_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcCall::set_allocated_call_data(::std::string* call_data) {
  if (call_data != NULL) {
    
  } else {
    
  }
  call_data_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), call_data);
  // @@protoc_insertion_point(field_set_allocated:nanorpc2.RpcCall.call_data)
}

// -------------------------------------------------------------------

// RpcResult

// optional .nanorpc2.RpcStatus status = 1;
inline void RpcResult::clear_status() {
  status_ = 0;
}
inline ::nanorpc2::RpcStatus RpcResult::status() const {
  // @@protoc_insertion_point(field_get:nanorpc2.RpcResult.status)
  return static_cast< ::nanorpc2::RpcStatus >(status_);
}
inline void RpcResult::set_status(::nanorpc2::RpcStatus value) {
  
  status_ = value;
  // @@protoc_insertion_point(field_set:nanorpc2.RpcResult.status)
}

// optional string error_message = 2;
inline void RpcResult::clear_error_message() {
  error_message_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& RpcResult::error_message() const {
  // @@protoc_insertion_point(field_get:nanorpc2.RpcResult.error_message)
  return error_message_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcResult::set_error_message(const ::std::string& value) {
  
  error_message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:nanorpc2.RpcResult.error_message)
}
inline void RpcResult::set_error_message(const char* value) {
  
  error_message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:nanorpc2.RpcResult.error_message)
}
inline void RpcResult::set_error_message(const char* value, size_t size) {
  
  error_message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:nanorpc2.RpcResult.error_message)
}
inline ::std::string* RpcResult::mutable_error_message() {
  
  // @@protoc_insertion_point(field_mutable:nanorpc2.RpcResult.error_message)
  return error_message_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcResult::release_error_message() {
  // @@protoc_insertion_point(field_release:nanorpc2.RpcResult.error_message)
  
  return error_message_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcResult::set_allocated_error_message(::std::string* error_message) {
  if (error_message != NULL) {
    
  } else {
    
  }
  error_message_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), error_message);
  // @@protoc_insertion_point(field_set_allocated:nanorpc2.RpcResult.error_message)
}

// optional bytes result_data = 3;
inline void RpcResult::clear_result_data() {
  result_data_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& RpcResult::result_data() const {
  // @@protoc_insertion_point(field_get:nanorpc2.RpcResult.result_data)
  return result_data_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcResult::set_result_data(const ::std::string& value) {
  
  result_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:nanorpc2.RpcResult.result_data)
}
inline void RpcResult::set_result_data(const char* value) {
  
  result_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:nanorpc2.RpcResult.result_data)
}
inline void RpcResult::set_result_data(const void* value, size_t size) {
  
  result_data_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:nanorpc2.RpcResult.result_data)
}
inline ::std::string* RpcResult::mutable_result_data() {
  
  // @@protoc_insertion_point(field_mutable:nanorpc2.RpcResult.result_data)
  return result_data_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcResult::release_result_data() {
  // @@protoc_insertion_point(field_release:nanorpc2.RpcResult.result_data)
  
  return result_data_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcResult::set_allocated_result_data(::std::string* result_data) {
  if (result_data != NULL) {
    
  } else {
    
  }
  result_data_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), result_data);
  // @@protoc_insertion_point(field_set_allocated:nanorpc2.RpcResult.result_data)
}

// -------------------------------------------------------------------

// RpcMessage

// optional int32 id = 1;
inline void RpcMessage::clear_id() {
  id_ = 0;
}
inline ::google::protobuf::int32 RpcMessage::id() const {
  // @@protoc_insertion_point(field_get:nanorpc2.RpcMessage.id)
  return id_;
}
inline void RpcMessage::set_id(::google::protobuf::int32 value) {
  
  id_ = value;
  // @@protoc_insertion_point(field_set:nanorpc2.RpcMessage.id)
}

// optional .nanorpc2.RpcCall call = 2;
inline bool RpcMessage::has_call() const {
  return !_is_default_instance_ && call_ != NULL;
}
inline void RpcMessage::clear_call() {
  if (GetArenaNoVirtual() == NULL && call_ != NULL) delete call_;
  call_ = NULL;
}
inline const ::nanorpc2::RpcCall& RpcMessage::call() const {
  // @@protoc_insertion_point(field_get:nanorpc2.RpcMessage.call)
  return call_ != NULL ? *call_ : *default_instance_->call_;
}
inline ::nanorpc2::RpcCall* RpcMessage::mutable_call() {
  
  if (call_ == NULL) {
    call_ = new ::nanorpc2::RpcCall;
  }
  // @@protoc_insertion_point(field_mutable:nanorpc2.RpcMessage.call)
  return call_;
}
inline ::nanorpc2::RpcCall* RpcMessage::release_call() {
  // @@protoc_insertion_point(field_release:nanorpc2.RpcMessage.call)
  
  ::nanorpc2::RpcCall* temp = call_;
  call_ = NULL;
  return temp;
}
inline void RpcMessage::set_allocated_call(::nanorpc2::RpcCall* call) {
  delete call_;
  call_ = call;
  if (call) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_set_allocated:nanorpc2.RpcMessage.call)
}

// optional .nanorpc2.RpcResult result = 3;
inline bool RpcMessage::has_result() const {
  return !_is_default_instance_ && result_ != NULL;
}
inline void RpcMessage::clear_result() {
  if (GetArenaNoVirtual() == NULL && result_ != NULL) delete result_;
  result_ = NULL;
}
inline const ::nanorpc2::RpcResult& RpcMessage::result() const {
  // @@protoc_insertion_point(field_get:nanorpc2.RpcMessage.result)
  return result_ != NULL ? *result_ : *default_instance_->result_;
}
inline ::nanorpc2::RpcResult* RpcMessage::mutable_result() {
  
  if (result_ == NULL) {
    result_ = new ::nanorpc2::RpcResult;
  }
  // @@protoc_insertion_point(field_mutable:nanorpc2.RpcMessage.result)
  return result_;
}
inline ::nanorpc2::RpcResult* RpcMessage::release_result() {
  // @@protoc_insertion_point(field_release:nanorpc2.RpcMessage.result)
  
  ::nanorpc2::RpcResult* temp = result_;
  result_ = NULL;
  return temp;
}
inline void RpcMessage::set_allocated_result(::nanorpc2::RpcResult* result) {
  delete result_;
  result_ = result;
  if (result) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_set_allocated:nanorpc2.RpcMessage.result)
}

// -------------------------------------------------------------------

// RpcVoid

// -------------------------------------------------------------------

// RpcObject

// optional uint32 object_id = 1;
inline void RpcObject::clear_object_id() {
  object_id_ = 0u;
}
inline ::google::protobuf::uint32 RpcObject::object_id() const {
  // @@protoc_insertion_point(field_get:nanorpc2.RpcObject.object_id)
  return object_id_;
}
inline void RpcObject::set_object_id(::google::protobuf::uint32 value) {
  
  object_id_ = value;
  // @@protoc_insertion_point(field_set:nanorpc2.RpcObject.object_id)
}

// -------------------------------------------------------------------

// RpcEvent

// optional string event_name = 1;
inline void RpcEvent::clear_event_name() {
  event_name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& RpcEvent::event_name() const {
  // @@protoc_insertion_point(field_get:nanorpc2.RpcEvent.event_name)
  return event_name_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcEvent::set_event_name(const ::std::string& value) {
  
  event_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:nanorpc2.RpcEvent.event_name)
}
inline void RpcEvent::set_event_name(const char* value) {
  
  event_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:nanorpc2.RpcEvent.event_name)
}
inline void RpcEvent::set_event_name(const char* value, size_t size) {
  
  event_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:nanorpc2.RpcEvent.event_name)
}
inline ::std::string* RpcEvent::mutable_event_name() {
  
  // @@protoc_insertion_point(field_mutable:nanorpc2.RpcEvent.event_name)
  return event_name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* RpcEvent::release_event_name() {
  // @@protoc_insertion_point(field_release:nanorpc2.RpcEvent.event_name)
  
  return event_name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void RpcEvent::set_allocated_event_name(::std::string* event_name) {
  if (event_name != NULL) {
    
  } else {
    
  }
  event_name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), event_name);
  // @@protoc_insertion_point(field_set_allocated:nanorpc2.RpcEvent.event_name)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace nanorpc2

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::nanorpc2::RpcStatus> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::nanorpc2::RpcStatus>() {
  return ::nanorpc2::RpcStatus_descriptor();
}

}  // namespace protobuf
}  // namespace google
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_rpc_5fproto_2frpc_5ftypes_2eproto__INCLUDED
