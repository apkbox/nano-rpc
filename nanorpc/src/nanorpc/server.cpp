#include "nanorpc/server.h"

#include <cassert>
#include <climits>
#include <memory>
#include <string>

namespace nanorpc {

SimpleServer::SimpleServer(std::unique_ptr<ServerTransport> transport)
    : transport_(std::move(transport)),
      services_(std::make_shared<ServiceManager>()) {}

SimpleServer::~SimpleServer() {
  try {
    Shutdown();
  }
  catch (...) {
  }
}

void SimpleServer::RegisterService(ServiceInterface *service) {
  assert(service != nullptr);
  if (service == nullptr)
    return;

  services_->AddService(service->GetInterfaceName(), service);
}

void SimpleServer::RegisterService(const std::string &name,
                                   ServiceInterface *service) {
  assert(service != nullptr);
  if (service == nullptr)
    return;

  services_->AddService(name, service);
}

bool SimpleServer::WaitForSingleRequest() {
  if (context_ == nullptr) {
    auto channel = transport_->Listen();
    if (channel == nullptr)
      return false;
    context_ = std::make_unique<ServerContext>(std::move(channel), services_);
  }

  return context_->WaitForSingleRequest();
}

bool SimpleServer::ConnectAndWait() {
  while (WaitForSingleRequest()) {
    /* EMPTY */;
  }
  return true;
}

void SimpleServer::Shutdown() {
  context_.release();
  transport_->Cancel();
}

bool SimpleServer::SendEvent(const RpcCall &call) {
  if (context_ == nullptr)
    return false;
  return context_->SendEvent(call);
}

Server::Context::Context(std::unique_ptr<ChannelInterface> &channel,
                         std::shared_ptr<ServiceManager> &service_manager)
    : ServerContext(channel, service_manager),
      thread_(&Context::ServerThread, this) {}

void Server::Context::Close() {
  ServerContext::Close();
  thread_.join();
}

void Server::Context::ServerThread() {
  while (WaitForSingleRequest()) {
    /* EMPTY */
  }
}

Server::Server(std::unique_ptr<ServerTransport> transport)
    : transport_(std::move(transport)),
    services_(std::make_shared<ServiceManager>()) {}

Server::~Server() {
  try {
    Shutdown();
  } catch (...) {
  }
}

void Server::RegisterService(ServiceInterface *service) {
  assert(service != nullptr);
  if (service == nullptr)
    return;

  services_->AddService(service->GetInterfaceName(), service);
}

void Server::RegisterService(const std::string &name,
                             ServiceInterface *service) {
  assert(service != nullptr);
  if (service == nullptr)
    return;

  services_->AddService(name, service);
}

bool Server::Wait() {
  // TODO: Prevent calling Wait concurrently. There could be only one thread.
  do {
    auto channel = transport_->Listen();
    if (channel != nullptr) {
      auto context = std::make_unique<Context>(std::move(channel), services_);
      contexts_.emplace_back(std::move(context));
    }
  } while (transport_->IsBound());

  return true;
}

void Server::Shutdown() {
  transport_->Cancel();
  for (auto &context : contexts_) {
    context->Close();
  }
}

bool Server::SendEvent(const RpcCall &call) {
  return true;
}

}  // namespace nanorpc
