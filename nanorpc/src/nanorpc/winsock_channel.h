#if !defined(NANORPC_WINSOCK_CHANNEL_H__)
#define NANORPC_WINSOCK_CHANNEL_H__

#include <memory>
#include <string>

#include "nanorpc/nanorpc2.h"

namespace nanorpc {

class WinsockChannelImpl;

class WinsockServerChannel final : public ServerChannelInterface {
public:
  WinsockServerChannel(const std::string &port);
  ~WinsockServerChannel();

  ChannelStatus GetStatus() const override;

  bool Connect() override;
  void Shutdown() override;
  void Disconnect() override;

  std::unique_ptr<ReadBuffer> Read(size_t bytes) override;
  std::unique_ptr<WriteBuffer> CreateWriteBuffer() override;
  void Write(std::unique_ptr<WriteBuffer> buffer) override;

  //bool Read(void *buffer, size_t buffer_size, size_t *bytes_read) override;
  //bool Write(void *buffer, size_t buffer_size) override;

private:
  std::unique_ptr<WinsockChannelImpl> impl_;

  NANORPC_DISALLOW_COPY_AND_ASSIGN(WinsockServerChannel);
};

class WinsockClientChannel final : public ClientChannelInterface {
public:
  WinsockClientChannel(const std::string &address, const std::string &port);
  ~WinsockClientChannel();

  ChannelStatus GetStatus() const override;

  bool Connect() override;
  void Shutdown() override;
  void Disconnect() override;

  std::unique_ptr<ReadBuffer> Read(size_t bytes) override;
  std::unique_ptr<WriteBuffer> CreateWriteBuffer() override;
  void Write(std::unique_ptr<WriteBuffer> buffer) override;

  //bool Read(void *buffer, size_t buffer_size, size_t *bytes_read) override;
  //bool Write(void *buffer, size_t buffer_size) override;

private:
  std::unique_ptr<WinsockChannelImpl> impl_;

  NANORPC_DISALLOW_COPY_AND_ASSIGN(WinsockClientChannel);
};

}  // namespace nanorpc

#endif  // NANORPC_WINSOCK_CHANNEL_H__
