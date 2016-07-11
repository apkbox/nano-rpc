#include "nanorpc/winsock_channel.h"
#include "nanorpc/winsock_channel_impl.h"
#include "nanorpc/winsock_server_transport_impl.h"

namespace nanorpc {

WinsockServerTransport::WinsockServerTransport(const std::string &port)
    : impl_{new WinsockServerTransportImpl{port}} {}

bool WinsockServerTransport::IsBound() const {
  return impl_->IsBound();
}

WinsockServerTransport::~WinsockServerTransport() {}

std::unique_ptr<ChannelInterface> WinsockServerTransport::Listen() {
  return std::unique_ptr<ChannelInterface>{impl_->Listen()};
}

void WinsockServerTransport::Cancel() {
  impl_->Cancel();
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
