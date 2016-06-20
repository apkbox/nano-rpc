#include <memory>
#include <thread>
#include <sstream>
#include <chrono>
#include <iostream>

#include "nanorpc/winsock_channel_impl.h"

#include <Windows.h>

void SendThread(nanorpc2::WinsockChannelImpl *channel) {
  const std::string kTestString = "TestString";

  int strsize = kTestString.size() + 1;

  for (int i = 0; i < 1000; ++i) {
    channel->Write(&strsize, sizeof(int));
    channel->Write(kTestString.c_str(), kTestString.size() + 1);
  }
}

void ReceiveThread(nanorpc2::WinsockChannelImpl *channel) {
  while (true) {
    int size;
    size_t read;
    bool result = channel->Read(&size, sizeof(int), &read);
    if (!result || (result && read == 0))
      break;

    std::unique_ptr<char[]> buffer(new char[size]);
    result = channel->Read(buffer.get(), size, &read);
    if (!result || (result && read == 0))
      break;
    std::cout << "Received: " << buffer.get() << std::endl;
  }
}

int wmain(int argc, wchar_t *argv[]) {
  nanorpc2::WinsockChannelImpl channel("localhost", "2345");

  for (int i = 0; i < 10; ++i) {
    std::cout << "CLIENT: Connecting... " << i << std::endl;
    if (channel.Connect()) {
      std::cout << "CLIENT: Connected!" << std::endl;
      break;
    }

    Sleep(1000);
  }

  std::thread send_thread(SendThread, &channel);
  std::thread receive_thread(ReceiveThread, &channel);

  send_thread.join();

  std::cout << "CLIENT: Exiting." << std::endl;
  channel.Shutdown();

  receive_thread.join();

  channel.Disconnect();

  return 0;
}
