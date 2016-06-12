#include <memory>
#include <thread>
#include <sstream>
#include <chrono>
#include <iostream>

#include "nanorpc/winsock_channel_impl.h"

#include <Windows.h>

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

  const std::string kTestString = "TestString";
  //auto request = channel.AllocateRequest();
  //auto buffer = request.AllocateBuffer(kTestString.size() + 1);
  //strcpy(buffer, kTestString.c_str());
  channel.Write(kTestString.c_str(), kTestString.size() + 1);

  std::cout << "CLIENT: Exiting." << std::endl;
  channel.Disconnect();

  return 0;
}
