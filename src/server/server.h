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
  
  void on_message(message::sequence_id sequence_id, const message::Pong& pong) {
    using namespace boost::pfr::ops;
    std::cout << "Unexpected message: " << pong << std::endl;
  }
  
  void on_message(message::sequence_id sequence_id, const message::Reply& reply) {
    using namespace boost::pfr::ops;
    std::cout << "Unexpected message: " << reply << std::endl;
  }
  
  void on_message(message::sequence_id sequence_id, const message::Ping& ping) {
    pong(sequence_id, ping);
  }
  
  void on_message(message::sequence_id sequence_id, const message::Request& request) {
    reply(sequence_id, request);
  }
  
  template <typename T>
  void send(message::sequence_id sequence_id, const T& message) {
    auto buffer = Serializer::serialize(sequence_id, message);
    SendAsync(buffer.first.get(), buffer.second);
  }
  
  template <typename T>
  void read(message::sequence_id sequence_id, const char* buffer, std::size_t buffer_size) {
    T message;
    prototype::Serializer::deserialize(buffer, buffer_size, message);
    on_message(sequence_id, message);
  }
  
  void onReceived(const void* buffer, size_t size) override
  {
    auto read_message = [this](const auto& header, const char* buffer) {
      switch (header.message_id) {
        case message::pong   : read<message::Pong>(header.sequence_id, buffer, header.size); break;
        case message::reply  : read<message::Reply>(header.sequence_id, buffer, header.size); break;
        case message::ping   : read<message::Ping>(header.sequence_id, buffer, header.size); break;
        case message::request: read<message::Request>(header.sequence_id, buffer, header.size); break;
      }
      return header.size;
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
