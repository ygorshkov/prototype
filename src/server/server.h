#pragma once

#include <iostream>

#include <boost/pfr.hpp>

#include <server/asio/tcp_server.h>

#include "message/ping.h"
#include "message/pong.h"
#include "message/request.h"
#include "message/reply.h"
#include "serialization/serializer.h"
#include "common/capacitor.h"

namespace prototype {
  
class Session : public CppServer::Asio::TCPSession
{
  enum { header_size = Serializer::header_size() };
  
public:
  using TCPSession::TCPSession;
  
private:
  void pong(const message::Ping& ping) {
    message::Pong pong{-ping.t, ping.t, 'X'};
    send(pong);
  }
  
  void reply(const message::Request& request) {
    message::Reply reply{request.id, request.id, request.e, 2.71f, request.a, request.c, static_cast<int8_t>(request.d)};
    send(reply);
  }
  
  void on_message(const message::Pong& pong) {
    using namespace boost::pfr::ops;
    std::cout << "Unexpected message: " << pong << std::endl;
  }
  
  void on_message(const message::Reply& reply) {
    using namespace boost::pfr::ops;
    std::cout << "Unexpected message: " << reply << std::endl;
  }
  
  void on_message(const message::Ping& ping) {
    pong(ping);
  }
  
  void on_message(const message::Request& request) {
    reply(request);
  }
  
  template <typename T>
  void send(const T& message) {
    auto buffer = Serializer::serialize(message);
    SendAsync(buffer.first.get(), buffer.second);
  }
  
  template <typename T>
  void read(const char* buffer, std::size_t buffer_size) {
    T message;
    prototype::Serializer::deserialize(buffer, buffer_size, message);
    on_message(message);
  }
  
  void onReceived(const void* buffer, size_t size) override
  {
    auto read_message = [this](const auto& header, const char* buffer) {
      switch (header.second) {
        case message::pong   : read<message::Pong>(buffer, header.first); break;
        case message::reply  : read<message::Reply>(buffer, header.first); break;
        case message::ping   : read<message::Ping>(buffer, header.first); break;
        case message::request: read<message::Request>(buffer, header.first); break;
      }
      return header.first;
    };
    
    auto head = static_cast<const char*>(buffer);
    if (capacitor) { // waiting for the rest of message
      auto consumed = capacitor->append(head, size);
      head += consumed;
      size -= consumed;
      
      if (0 == capacitor->required()) {
        read_message(capacitor->header, capacitor->buffer().first.get());
        capacitor = nullptr;
      }
    }
    
    while (size >= header_size) {
      const auto header = Serializer::read_header(head, header_size);
      head += header_size;
      size -= header_size;
      
      if (size < header.first) {
        capacitor = std::make_unique<Capacitor>(header, head, size);
        break;
      }
      
      auto read = read_message(header, head);
      head += read;
      size -= read;
    }
  }
  
  void onError(int error, const std::string& category, const std::string& message) override {
    std::cout << "TCP session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
  }
  
  std::unique_ptr<Capacitor> capacitor;
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
