#pragma once

#include <iostream>

#include <boost/pfr.hpp>

#include <server/asio/tcp_server.h>

#include "message/ping.h"
#include "message/pong.h"
#include "message/request.h"
#include "message/reply.h"
#include "serialization/serializer.h"
#include "common/receiver.h"
#include "common/protocol.h"

namespace prototype {
  
class Session : public CppServer::Asio::TCPSession
{
  enum { header_size = Serializer::header_size() };
  
public:
  using TCPSession::TCPSession;
  
private:
  void pong(message::sequence_id sequence_id, const message::Ping& ping) {
    message::Pong pong{-ping.t, ping.t, 'X'};
    send(sequence_id, pong);
  }
  
  void reply(message::sequence_id sequence_id, const message::Request& request) {
    message::Reply reply{request.id, request.id, request.e, 2.71f, request.a, request.c, static_cast<int8_t>(request.d)};
    send(sequence_id, reply);
  }
  
  template <typename T>
  void on_message(message::sequence_id sequence_id, const T& message) {
    using namespace boost::pfr::ops;
    std::cout << "Unhandled message: " << message << std::endl;
  }
  
  template <typename T>
  void send(message::sequence_id sequence_id, const T& message) {
    auto buffer = Serializer::serialize(sequence_id, message);
    SendAsync(buffer.first.get(), buffer.second);
  }

  void onReceived(const void* buffer, size_t size) override
  {
    auto read_message = [this](const auto& header, const char* data) {
      return dispatch(header, data)(
        on<message::Ping>([this](auto sequence_id, auto message) { pong(sequence_id, message); }),
        on<message::Pong>([this](auto sequence_id, auto message) { on_message(sequence_id, message); }),
        on<message::Request>([this](auto sequence_id, auto message) { reply(sequence_id, message); }),
        on<message::Reply>([this](auto sequence_id, auto message) { on_message(sequence_id, message); }),
        other([this](auto header, auto buffer) { onError(42, "protocol error", "unknown message type"); }));
    };
    
    receiver.on_received(buffer, size, read_message);
  }
  
  void onError(int error, const std::string& category, const std::string& message) override {
    std::cout << "TCP session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
  }
  
  Receiver receiver;
};

class Server final : public CppServer::Asio::TCPServer
{
public:
  using CppServer::Asio::TCPServer::TCPServer;
  
protected:
  std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<CppServer::Asio::TCPServer>& server) override {
    return std::make_shared<Session>(server);
  }
  
protected:
  void onError(int error, const std::string& category, const std::string& message) override {
    std::cout << "TCP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
  }
};
  
}
