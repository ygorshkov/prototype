#pragma once

#include <memory>
#include <cstddef>

#include "capacitor.h"
#include "serialization/serializer.h"

namespace prototype {

struct Receiver {
  enum { header_size = Serializer::header_size() };

  template <typename F>
  void on_received(const void* buffer, size_t size, F read_message) {
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
  
  std::unique_ptr<Capacitor> capacitor = nullptr;
};

}
