#pragma once

#include <algorithm>
#include <utility>
#include <cstddef>

#include "message/id.h"
#include "serialization/serializer.h"

namespace prototype {

/// Holds incomplete buffer
class Capacitor{
public:
  const Header header;
  
  Capacitor(Header _header, const char* buffer, size_t size)
      : header{_header},
        buffer_{allocate_buffer(header.size)},
        required_{header.size}
  {
    append(buffer, size);
  }
  
  std::uint32_t required() const { return required_; }
  buffer_pair buffer() const { return buffer_; }
  
  std::uint32_t append(const char* buffer, size_t size) {
    auto consume = std::min(size, required_);
    std::copy(buffer, buffer + consume, buffer_.first.get() + header.size - required_);
    required_ -= consume;
    return consume;
  }
  
private:
  buffer_pair buffer_;
  std::size_t required_;
};

}
