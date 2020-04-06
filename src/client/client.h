#pragma once

#include <iostream>
#include <memory>
#include <array>

#include <server/asio/tcp_client.h>

#include "message/ping.h"
#include "message/pong.h"
#include "message/request.h"
#include "message/reply.h"
#include "message/id.h"

#include "serialization/serializer.h"
#include "common/receiver.h"
#include "common/protocol.h"
#include "common/measure.h"

std::atomic<uint64_t> total_errors(0);

namespace prototype {

class Client : public CppServer::Asio::TCPClient
{
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
        std::terminate();
      }
      return seq_id;
    }
    
    std::atomic<message::sequence_id> sequence_id = 0;
    mutable std::array<Callback, max_sequence_id + 1> callbacks;
  };
  
  Callbacks<void(const message::Pong&)> on_pongs_;
  Callbacks<void(const message::Reply&)> on_replies_;

public:
  Client(const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port)
      : TCPClient(service, address, port)
  {}
 
  Measurer serialization{"serialization"};
  
  template <typename F>
  void ping(const message::Ping& ping, F on_pong) {
    send(on_pongs_.add_callback(on_pong), ping, serialization);
  }

  template <typename F>
  void request(const message::Request& request, F on_reply) {
    send(on_replies_.add_callback(on_reply), request, serialization);
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
      Measure m{&measurer};
      buffer = Serializer::serialize(sequence_id, message);
    }
    SendAsync(buffer.first.get(), buffer.second);
  }

  void onConnected() override {}

  void onSent(size_t sent, size_t pending) override {}

  void onReceived(const void* buffer, size_t size) override
  {
    receiver.on_received(buffer, size, [this](const Header& header, const char* data) {
      return dispatch(header, data, &serialization)(
        on<message::Ping>([this](auto sequence_id, auto message) { on_message(sequence_id, message); }),
        on<message::Pong>([this](auto sequence_id, auto message) { on_message(sequence_id, message); }),
        on<message::Request>([this](auto sequence_id, auto message) { on_message(sequence_id, message); }),
        on<message::Reply>([this](auto sequence_id, auto message) { on_message(sequence_id, message); }),
        other([this](auto header, auto buffer) { onError(42, "protocol error", "unknown message type"); }));
    });
  }

  void onError(int error, const std::string& category, const std::string& message) override
  {
    std::cout << "TCP client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
  }
  
  Receiver receiver;
};

}
