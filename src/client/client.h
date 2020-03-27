#pragma once

#include <iostream>
#include <memory>

#include <boost/pfr.hpp>
#include <server/asio/tcp_client.h>

#include "message/ping.h"
#include "message/pong.h"
#include "message/request.h"
#include "message/reply.h"

#include "serialization/serializer.h"
#include "common/capacitor.h"
#include "measure.h"

std::atomic<uint64_t> total_errors(0);

namespace prototype {

template <typename Factory>
class Client : public CppServer::Asio::TCPClient
{
  enum { header_size = Serializer::header_size() };

public:
  Client(const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port, bool verbose = false)
    : TCPClient(service, address, port), verbose_{verbose}
  {}
 
  Measurer ping_pong {"ping - pong round trip"};
  Measurer request_reply {"request - reply round trip"};
  Measurer ping_pong_ser {"ping - pong serialization"};
  Measurer request_reply_ser {"request - reply serialization"};
  
  void ping() {
    auto ping = Factory::create_ping();
    current_ping_pong = std::make_unique<Measure>(ping_pong);
    send(ping, ping_pong_ser);
  }

  void request() {
    auto request = Factory::create_request();
    current_request_reply = std::make_unique<Measure>(request_reply);
    send(request, request_reply_ser);
  }
  
private:
  void on_message(const prototype::message::Pong& pong) {
    current_ping_pong = nullptr;
    request();
  }
         
  void on_message(const message::Reply& reply) {
    current_request_reply = nullptr;
    ping();
  }

  void on_message(const message::Ping& ping) {
    using namespace boost::pfr::ops;
    std::cout << "Unexpected message: " << ping << std::endl;
  }
  
  void on_message(const message::Request& request) {
    using namespace boost::pfr::ops;
    std::cout << "Unexpected message: " << request << std::endl;
  }

  template <typename T>
  void send(const T& message, Measurer& measurer) {
    buffer_pair buffer;
    {
      Measure m{measurer};
      buffer = Serializer::serialize(message);
    }
    SendAsync(buffer.first.get(), buffer.second);
  }

  template <typename T>
  void read(const char* buffer, std::size_t buffer_size, Measurer& measurer) {
    T message;
    {
      Measure m{measurer};
      prototype::Serializer::deserialize(buffer, buffer_size, message);
    }
    
    if (verbose_) {
      using namespace boost::pfr::ops;
      std::cout << message << '\n';
    }
    
    on_message(message);
  }
  
  void onConnected() override { ping(); }

  void onSent(size_t sent, size_t pending) override {}
                                                               
  void onReceived(const void* buffer, size_t size) override
  {
    auto read_message = [this](const auto& header, const char* buffer) {
      switch (header.second) {
        case message::pong   : read<message::Pong>(buffer, header.first, ping_pong_ser); break;
        case message::reply  : read<message::Reply>(buffer, header.first, request_reply_ser); break;
        case message::ping   : read<message::Ping>(buffer, header.first, ping_pong_ser); break;
        case message::request: read<message::Request>(buffer, header.first, request_reply_ser); break;
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

  void onError(int error, const std::string& category, const std::string& message) override
  {
    std::cout << "TCP client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
  }
  
  std::unique_ptr<Capacitor> capacitor;
  std::unique_ptr<Measure> current_ping_pong;
  std::unique_ptr<Measure> current_request_reply;
  bool verbose_ = false;
};

}
