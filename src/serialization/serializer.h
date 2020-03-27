#pragma once

#include "message/id.h"

#include <yas/mem_streams.hpp>
#include <yas/binary_iarchive.hpp>
#include <yas/binary_oarchive.hpp>
#include <yas/std_types.hpp>

#include <boost/pfr.hpp>

#include <memory>
#include <utility>

namespace prototype {

using buffer_ptr  = std::shared_ptr<char>;
using buffer_pair = std::pair<buffer_ptr, std::size_t>;

buffer_pair allocate_buffer(const std::size_t size) {
  buffer_ptr buffer(new char[size], [](char *p){delete[] p;});
  return {std::move(buffer), size};
}

class Serializer {
public:
  template <typename Message>
  static buffer_pair serialize(const Message& message) {
    auto ostream = yas::mem_ostream{};
    reserve_space_for_header(ostream);
    
    auto oarchive = yas::binary_oarchive<yas::mem_ostream>{ostream};
    const auto tuple = boost::pfr::structure_to_tuple(message);
    oarchive & tuple;

    const auto& intrusive_buffer = ostream.get_intrusive_buffer();
    write_header(Message::message_id, intrusive_buffer.data, intrusive_buffer.size);
    
    const auto& shared_buffer = ostream.get_shared_buffer();
    return {shared_buffer.data, shared_buffer.size};
  }
  
  template <typename Message>
  static void deserialize(const char* buffer, std::size_t buffer_size, Message& message) {
    auto istream = yas::mem_istream{buffer, buffer_size};
    auto iarchive = yas::binary_iarchive<yas::mem_istream>{istream};
    auto tuple = boost::pfr::structure_tie(message);
    iarchive & tuple;
  }

  static constexpr std::size_t header_size() {
    static_assert(sizeof(std::uint32_t) == sizeof(message::id), "bad types");
    return sizeof(std::uint32_t) + sizeof(message::id);
  }

  template <typename OS>
  static void reserve_space_for_header(OS& ostream) {
    static const char header_buffer[header_size()] = "\0";
    ostream.write(header_buffer, header_size());
  }

  static void write_header(message::id message_id, const char* ptr, std::size_t size) {
    auto* header = (std::uint32_t*)ptr;
    header[0] = htonl((std::uint32_t)(size - header_size()));
    header[1] = htonl(message_id);
  }
  
  static auto read_header(const char* ptr, std::size_t/* size*/) {
    const auto* header = (const std::uint32_t*)ptr;
    return std::pair<std::uint32_t, typename message::id>{
      ntohl(header[0]),
      static_cast<typename message::id>(ntohl(header[1]))
    };
  }
};

}
