#pragma once

#include <boost/hana.hpp>

#include "serialization/serializer.h"
#include "message/id.h"
#include "measure.h"

namespace hana = boost::hana;

namespace prototype {

template <typename T>
auto on = [](auto f) {
  return hana::make_pair(hana::type_c<T>, f);
};

struct Other;
auto other = on<Other>;

template <typename OtherT>
  auto process(const Header& header, const char* buffer, Measurer* measurer, OtherT& other) {
  other(header, buffer);
  return header.size;
}

template <typename OtherT, typename OnT, typename ...Rest>
auto process(const Header& header, const char* buffer, Measurer* measurer, OtherT& other, OnT& on, Rest& ...rest)
{
  using T = typename decltype(+hana::first(on))::type;

  if (header.message_id == T::message_id) {
    T message;
    {
      Measure m{measurer};
      prototype::Serializer::deserialize(buffer, header.size, message);
    }
    hana::second(on)(header.sequence_id, message);
    return header.size;
  }
  return process(header, buffer, measurer, other, rest...);
}

auto dispatch(const Header& header, const char* buffer, Measurer* serialization_measurer = nullptr) {
  return [&header, buffer, measurer = serialization_measurer](auto ...handlers_) mutable {
    auto handlers = hana::make_tuple(handlers_...);
    auto other = hana::find_if(handlers, [](auto const& c) {
      return hana::first(c) == hana::type_c<Other>;
    });
    static_assert(other != hana::nothing, "dispatch must have other");
    auto rest = hana::filter(handlers, [](auto const& c) {
      return hana::first(c) != hana::type_c<Other>;
    });
    return hana::unpack(rest, [&](auto& ...rest) {
      return process(header, buffer, measurer, hana::second(*other), rest...);
    });
  };
}

}
