#include "nanorpc/winsock_channel.h"

#include "nanorpc/winsock_channel_impl.h"

namespace nanorpc {

WinsockServerChannel::WinsockServerChannel(const std::string &port)
    : impl_{ new WinsockChannelImpl{ port } } {}

WinsockServerChannel::~WinsockServerChannel() {}

ChannelStatus WinsockServerChannel::GetStatus() const {
  return impl_->GetStatus();
}

bool WinsockServerChannel::Connect() {
  return impl_->Connect();
}

void WinsockServerChannel::Shutdown() {
  impl_->Shutdown();
}

void WinsockServerChannel::Disconnect() {
  impl_->Disconnect();
}

std::unique_ptr<ReadBuffer> WinsockServerChannel::Read(size_t bytes) {
  return impl_->Read(bytes);
}

std::unique_ptr<WriteBuffer> WinsockServerChannel::CreateWriteBuffer() {
  return impl_->CreateWriteBuffer();
}

void WinsockServerChannel::Write(std::unique_ptr<WriteBuffer> buffer) {
  impl_->Write(std::move(buffer));
}

WinsockClientChannel::WinsockClientChannel(const std::string &address,
                                           const std::string &port)
    : impl_{ new WinsockChannelImpl{ address, port } } {}

WinsockClientChannel::~WinsockClientChannel() {}

ChannelStatus WinsockClientChannel::GetStatus() const {
  return impl_->GetStatus();
}

bool WinsockClientChannel::Connect() {
  return impl_->Connect();
}

void WinsockClientChannel::Shutdown() {
  impl_->Shutdown();
}

void WinsockClientChannel::Disconnect() {
  impl_->Disconnect();
}

std::unique_ptr<ReadBuffer> WinsockClientChannel::Read(size_t bytes) {
  return impl_->Read(bytes);
}

std::unique_ptr<WriteBuffer> WinsockClientChannel::CreateWriteBuffer() {
  return impl_->CreateWriteBuffer();
}

void WinsockClientChannel::Write(std::unique_ptr<WriteBuffer> buffer) {
  impl_->Write(std::move(buffer));
}

}  // namespace nanorpc
