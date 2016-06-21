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
  int a;
  int n_seq;
};

struct CalcSequence {
  Operator op;
  int b;
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
    auto request_size = channel.Read(sizeof(int));
    if (request_size == nullptr)
      break;

    auto request_message = channel.Read(*request_size->ReadAs<int>());
    if (request_message == nullptr)
      break;

    const RequestMessage *request = request_message->ReadAs<RequestMessage>();

    std::cout << "SERVER: [" << request->call_id
              << "]: Request with a=" << request->a
              << ", n_seq=" << request->n_seq << std::endl;

    std::unique_ptr<nanorpc2::WriteBuffer> response_message = channel.CreateWriteBuffer();
    int *response_size = response_message->WriteAs<int>();
    ResponseMessage *response = response_message->WriteAs<ResponseMessage>();
    response->call_id = request->call_id;
    response->result = request->a;

    for (int n = 0; n < request->n_seq; ++n) {
      const CalcSequence *calc = request_message->ReadAs<CalcSequence>();
      if (calc->op == Operator::Add)
        response->result += calc->b;
      else if (calc->op == Operator::Subtract)
        response->result -= calc->b;
    }

    std::cout << "SERVER: [" << request->call_id
              << "]: Result=" << response->result << std::endl;

    *response_size = response_message->size() - sizeof(int);
    channel.Write(std::move(response_message));

    std::cout << "SERVER: [" << request->call_id << "]: Done." << std::endl;
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
  std::uniform_int_distribution<> seq_gen{ 1, 5 };
  std::uniform_int_distribution<> ab_gen{ 0, 200 };
  std::uniform_int_distribution<> op_gen{ 1, 2 };

  for (int call_id = 0; call_id < 10; ++call_id) {
    auto a = ab_gen(rnd_generator);
    auto n_seq = seq_gen(rnd_generator);

    RequestMessage request;
    request.call_id = call_id;
    request.a = a;
    request.n_seq = n_seq;

    std::cout << "CLIENT: [" << request.call_id
              << "]: Request with a=" << request.a
              << ", n_seq=" << request.n_seq << std::endl;

    int message_size = sizeof(RequestMessage) + sizeof(CalcSequence) * n_seq;
    if (!channel.Write(&message_size, sizeof(int)))
      break;
    if (!channel.Write(&request, sizeof(RequestMessage)))
      break;

    for (int n = 0; n < n_seq; ++n) {
      auto b = ab_gen(rnd_generator);
      auto op = op_gen(rnd_generator);

      CalcSequence seq;
      seq.op = OperatorFromInt(op);
      seq.b = b;
      if (!channel.Write(&seq, sizeof(CalcSequence)))
        break;
    }

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
