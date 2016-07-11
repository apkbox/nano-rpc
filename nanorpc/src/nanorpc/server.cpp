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
  } catch (...) {
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

EventSourceInterface *SimpleServer::GetEventSource() {
  return context_.get();
}

Server::Server(std::unique_ptr<ServerTransport> transport)
    : transport_(std::move(transport)),
      services_(std::make_shared<ServiceManager>()),
      event_source_(*this) {}

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

EventSourceInterface *Server::GetEventSource() {
  return &event_source_;
}

Server::Context::Context(std::unique_ptr<ChannelInterface> &channel,
                         std::shared_ptr<ServiceManager> &service_manager)
    : ServerContext(channel, service_manager),
      thread_(&Context::ServerThread, this) {}

void Server::Context::Close() {
  ServerContext::Close();
  thread_.join();
}

EventSourceInterface *Server::Context::GetEventSource() {
  return this;
}

void Server::Context::ServerThread() {
  while (WaitForSingleRequest()) {
    /* EMPTY */
  }
}

bool Server::EventSource::SendEvent(const RpcCall &call) {
  bool result = false;
  for (auto &c : server_.contexts_)
    result |= c->SendEvent(call);
  return false;
}

}  // namespace nanorpc
