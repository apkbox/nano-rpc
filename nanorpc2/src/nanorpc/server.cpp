#include "nanorpc/nanorpc2.h"

#include <cassert>
#include <climits>
#include <memory>
#include <string>

namespace nanorpc2 {

Server::Server(std::unique_ptr<ServerChannelInterface> channel)
    : channel_(std::move(channel)) {}

void Server::RegisterService(ServiceInterface *service) {
  assert(service != nullptr);
  if (service == nullptr)
    return;

  object_manager_.AddService(service->GetInterfaceName(), service);
}

void Server::RegisterService(const std::string &name,
                             ServiceInterface *service) {
  assert(service != nullptr);
  if (service == nullptr)
    return;

  object_manager_.AddService(name, service);
}

bool Server::WaitForSingleRequest() {
  if (channel_->GetStatus() != ChannelStatus::Established) {
    if (!channel_->Connect())
      return false;
  }

  auto rdbuf = channel_->Read(sizeof(uint32_t));
  if (rdbuf == nullptr)
    return false;

  uint32_t message_size;
  if (!rdbuf->ReadAs(&message_size))
    return false;

  rdbuf = channel_->Read(message_size);
  if (rdbuf == nullptr)
    return false;

  RpcMessage request;
#pragma warning(suppress : 4267)
  request.ParseFromArray(rdbuf->Read(message_size), message_size);

  RpcMessage response;
  bool is_async = false;
  ProcessRequest(request, &response, &is_async);

  if (!is_async) {
    auto wrbuf = channel_->CreateWriteBuffer();
    auto response_size = response.ByteSize();
    wrbuf->WriteAs<uint32_t>(response_size);
    response.SerializeToArray(wrbuf->Write(response_size), response_size);
    channel_->Write(std::move(wrbuf));
  }

  return true;
}

bool Server::ConnectAndWait() {
  return false;
}

void Server::Shutdown() {}

void Server::ProcessRequest(const RpcMessage &request,
                            RpcMessage *response,
                            bool *is_async) {
  assert(response != nullptr);
  assert(is_async != nullptr);
  if (response == nullptr || is_async == nullptr)
    return;

  assert(!request.has_result());
  if (request.has_result())
    return;

  ServiceInterface *service = nullptr;
  response->set_id(request.id());
  *is_async = false;

  // Check whether the called method is for service or transient object.
  // The service are ideentified by name and lifetime is controlled by the
  // server side.
  // The transient objects are registered as result of a method call and
  // identified by object ID. The lifetime of transient object is
  // controlled by the client.
  if (request.call().object_id() != 0) {
    // Try to find object that corresponds to the marshalled interface.
    service = object_manager_.GetObject(request.call().object_id());
    if (service == nullptr) {
      response->mutable_result()->set_status(RpcUnknownInterface);
      response->mutable_result()->set_error_message(
          "Marshalled object does not exist (was object disposed?).");
    }
  } else {
    // Try to find service.
    service = object_manager_.GetService(request.call().service().c_str());
    if (service == nullptr) {
      response->mutable_result()->set_status(RpcUnknownInterface);
      response->mutable_result()->set_error_message("Unknown interface");
    }
  }

  if (service != nullptr) {
    RpcResult rpc_result;
    if (service->CallMethod(request.call(), &rpc_result))
      response->mutable_result()->MergeFrom(rpc_result);
    else
      *is_async = true;
  } else {
    assert(response->has_result());
  }
}

}  // namespace nanorpc2
