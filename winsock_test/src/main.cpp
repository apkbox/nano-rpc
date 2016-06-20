#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <thread>

#include "nanorpc/winsock_channel_impl.h"

#include <Windows.h>

enum class Operator {
  Nop = 0,
  Add = 1,
  Subtract = 2
};

Operator OperatorFromInt(int value) {
  if (value < 1)
    return Operator::Nop;
  if (value > 2)
    return Operator::Nop;

  return (Operator)value;
}

struct RequestMessage {
  int call_id;
  Operator op;
  int operand1;
  int operand2;
};

struct ResponseMessage {
  int call_id;
  int result;
};

void ServerThreadProc() {
  std::cout << "SERVER: Starting... " << std::endl;
  nanorpc2::WinsockChannelImpl channel("2345");
  if (!channel.Connect()) {
    std::cout << "SERVER: Connection failed." << std::endl;
    return;
  }

  while (true) {
    int request_size;
    size_t bytes_read;
    if (!channel.Read(&request_size, sizeof(int), &bytes_read))
      break;
    if (bytes_read == 0)
      break;

    RequestMessage request;
    if (!channel.Read(&request, request_size, &bytes_read))
      break;
    if (bytes_read == 0)
      break;

    std::cout << "SERVER: [" << request.call_id << "]: Op=" << (int)request.op
              << ", a=" << request.operand1 << ", b=" << request.operand2
              << std::endl;

    ResponseMessage response;
    response.call_id = request.call_id;
    if (request.op == Operator::Add) {
      response.result = request.operand1 + request.operand2;
    } else if (request.op == Operator::Subtract) {
      response.result = request.operand1 - request.operand2;
    } else { 
      response.result = -1;
    }

    std::cout << "SERVER: [" << request.call_id
              << "]: Result=" << response.result << std::endl;

    int response_size = sizeof(ResponseMessage);
    if (!channel.Write(&response_size, sizeof(int)))
      break;
    if (!channel.Write(&response, sizeof(ResponseMessage)))
      break;

    std::cout << "SERVER: [" << request.call_id << "]: Done." << std::endl;
  }
  
  channel.Disconnect();

  std::cout << "SERVER: Exiting." << std::endl;
}

void ClientThreadProc() {
  std::cout << "CLIENT: Starting... " << std::endl;
  nanorpc2::WinsockChannelImpl channel("localhost", "2345");

  for (int i = 0; i < 10; ++i) {
    std::cout << "CLIENT: Connecting... " << i << std::endl;
    if (channel.Connect()) {
      std::cout << "CLIENT: Connected!" << std::endl;
      break;
    }

    Sleep(1000);
  }

  std::random_device rd;
  std::mt19937 rnd_generator{ rd() };
  std::uniform_int_distribution<> ab_gen{ 0, 200 };
  std::uniform_int_distribution<> op_gen{ 1, 2 };

  for (int call_id = 0; call_id < 10; ++call_id) {
    auto a = ab_gen(rnd_generator);
    auto b = ab_gen(rnd_generator);
    auto op = op_gen(rnd_generator);
    RequestMessage request;
    request.call_id = call_id;
    request.op = OperatorFromInt(op);
    request.operand1 = a;
    request.operand2 = b;

    std::cout << "CLIENT: [" << request.call_id << "]: Op=" << (int)request.op
              << ", a=" << request.operand1 << ", b=" << request.operand2
              << std::endl;

    int message_size = sizeof(RequestMessage);
    if (!channel.Write(&message_size, sizeof(int)))
      break;

    if (!channel.Write(&request, message_size))
      break;

    int response_size;
    size_t bytes_read;
    if (!channel.Read(&response_size, sizeof(int), &bytes_read))
      break;
    if (bytes_read == 0)
      break;

    std::vector<uint8_t> buffer(response_size);
    if (!channel.Read(&buffer[0], buffer.size(), &bytes_read))
      break;
    if (bytes_read == 0)
      break;

    if (bytes_read < sizeof(ResponseMessage))
      break;

    ResponseMessage response;
    memcpy(&response, &buffer[0], sizeof(ResponseMessage));
    if (response.call_id != call_id)
      break;

    std::cout << "CLIENT: [" << call_id << "]: Result=" << response.result
              << std::endl;
  }

  channel.Shutdown();

  std::cout << "CLIENT: Exiting." << std::endl;
}

int wmain(int argc, wchar_t *argv[]) {
  std::thread server_thread(ServerThreadProc);
  std::thread client_thread(ClientThreadProc);

  client_thread.join();
  server_thread.join();

  return 0;
}
