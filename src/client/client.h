#pragma once

#include <iostream>
#include <memory>
#include <array>

#include <boost/pfr.hpp>
#include <server/asio/tcp_client.h>

#include "message/ping.h"
#include "message/pong.h"
#include "message/request.h"
#include "message/reply.h"
#include "message/id.h"

#include "serialization/serializer.h"
#include "common/capacitor.h"
#include "measure.h"

std::atomic<uint64_t> total_errors(0);

namespace prototype {

class Client : public CppServer::Asio::TCPClient
{
  enum { header_size = Serializer::header_size() };

  template <typename F>
  struct Callbacks {
    using Callback = std::function<F>;
    
    static const message::sequence_id max_sequence_id = 1024 - 1;
    
    message::sequence_id add_callback(std::function<F> f) {
      auto seq_id = ++sequence_id & max_sequence_id;
      if (callbacks[seq_id]) {
        std::cout << "Too much simultaneous messages." << seq_id << '\n';
        get_next_sequence_id(seq_id);
      }
      callbacks[seq_id] = f;
      return seq_id;
    }
    
    template<typename T>
    auto operator()(message::sequence_id sequence_id, const T& arg) {
      return get_callback(sequence_id)(arg);
    }
    
  private:
   auto get_callback(message::sequence_id sequence_id) const {
      auto f = callbacks[sequence_id];
      Callback empty{};
      callbacks[sequence_id].swap(empty);
      return f;
    }

    message::sequence_id get_next_sequence_id(message::sequence_id seq_id) {
      while (callbacks[seq_id]) { // spin lock sending thread
        // TODO: do something here
        //seq_id = ++sequence_id & max_sequence_id;
      }
      return seq_id;
    }
    
    std::atomic<message::sequence_id> sequence_id = 0;
    mutable std::array<Callback, max_sequence_id + 1> callbacks;
  };
  
  Callbacks<void(const message::Pong&)> on_pongs_;
  Callbacks<void(const message::Reply&)> on_replies_;

public:
  Client(const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port, bool verbose = false)
      : TCPClient(service, address, port), verbose_{verbose}
  {}
 
  Measurer ping_pong_ser {"ping - pong serialization"};
  Measurer request_reply_ser {"request - reply serialization"};
  
  template <typename F>
  void ping(const message::Ping& ping, F on_pong) {
    send(on_pongs_.add_callback(on_pong), ping, ping_pong_ser);
  }

  template <typename F>
  void request(const message::Request& request, F on_reply) {
    send(on_replies_.add_callback(on_reply), request, request_reply_ser);
  }
  
private:
  void on_message(message::sequence_id sequence_id, const prototype::message::Pong& pong) {
    on_pongs_(sequence_id, pong);
  }
         
  void on_message(message::sequence_id sequence_id, const message::Reply& reply) {
    on_replies_(sequence_id, reply);
  }

  void on_message(message::sequence_id sequence_id, const message::Ping& ping) {
    using namespace boost::pfr::ops;
    std::cout << "Unexpected message: " << ping << std::endl;
  }
  
  void on_message(message::sequence_id sequence_id, const message::Request& request) {
    using namespace boost::pfr::ops;
    std::cout << "Unexpected message: " << request << std::endl;
  }

  template <typename T>
  void send(message::sequence_id sequence_id, const T& message, Measurer& measurer) {
    buffer_pair buffer;
    {
      Measure m{measurer};
      buffer = Serializer::serialize(sequence_id, message);
    }
    SendAsync(buffer.first.get(), buffer.second);
  }

  template <typename T>
  void read(message::sequence_id sequence_id, const char* buffer, std::size_t buffer_size, Measurer& measurer) {
    T message;
    {
      Measure m{measurer};
      prototype::Serializer::deserialize(buffer, buffer_size, message);
    }
    
    if (verbose_) {
      using namespace boost::pfr::ops;
      std::cout << message << '\n';
    }
    
    on_message(sequence_id, message);
  }
  
  void onConnected() override {}

  void onSent(size_t sent, size_t pending) override {}
                                                               
  void onReceived(const void* buffer, size_t size) override
  {
    auto read_message = [this](const auto& header, const char* buffer) {
      switch (header.message_id) {
        case message::pong   : read<message::Pong>(header.sequence_id, buffer, header.size, ping_pong_ser); break;
        case message::reply  : read<message::Reply>(header.sequence_id, buffer, header.size, request_reply_ser); break;
        case message::ping   : read<message::Ping>(header.sequence_id, buffer, header.size, ping_pong_ser); break;
        case message::request: read<message::Request>(header.sequence_id, buffer, header.size, request_reply_ser); break;
      }
      return header.size;
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

      if (size < header.size) {
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

  bool verbose_ = false;
};

}
