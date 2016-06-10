#include "nanorpc/winsock_channel.h"

#include "nanorpc/winsock_channel_impl.h"

namespace nanorpc2 {

WinsockServerChannel::WinsockServerChannel(const std::string &port)
    : impl_{ new WinsockServerChannel::Impl{ port } } {}

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

}  // namespace nanorpc2
