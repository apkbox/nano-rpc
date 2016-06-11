#include "nanorpc/winsock_channel.h"

#include "nanorpc/winsock_channel_impl.h"

namespace nanorpc2 {

WinsockServerChannel::WinsockServerChannel(const std::string &port)
    : impl_{ new WinsockChannelImpl{ port } } {}

WinsockServerChannel::~WinsockServerChannel() {}

bool WinsockServerChannel::WaitForClient() {
  return false;
}

bool WinsockServerChannel::WaitForClientAsync() {
  return false;
}

bool WinsockServerChannel::Connect() {
  return impl_->Connect();
}
void WinsockServerChannel::Disconnect() {
  impl_->Disconnect();
}

bool WinsockServerChannel::Read(void *buffer,
                                size_t buffer_size,
                                size_t *bytes_read) {
  return false;
}

bool WinsockServerChannel::Write(void *buffer, size_t buffer_size) {
  return false;
}

ChannelStatus WinsockServerChannel::GetStatus() const {
  return impl_->GetStatus();
}

WinsockClientChannel::WinsockClientChannel(const std::string &address,
                                           const std::string &port)
    : impl_{ new WinsockChannelImpl{ address, port } } {}

WinsockClientChannel::~WinsockClientChannel() {}

bool WinsockClientChannel::WaitForClient() {
  return false;
}

bool WinsockClientChannel::WaitForClientAsync() {
  return false;
}

bool WinsockClientChannel::Connect() {
  return impl_->Connect();
}
void WinsockClientChannel::Disconnect() {
  impl_->Disconnect();
}

bool WinsockClientChannel::Read(void *buffer,
                                size_t buffer_size,
                                size_t *bytes_read) {
  return false;
}

bool WinsockClientChannel::Write(void *buffer, size_t buffer_size) {
  return false;
}

ChannelStatus WinsockClientChannel::GetStatus() const {
  return impl_->GetStatus();
}

}  // namespace nanorpc2
