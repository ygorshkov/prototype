#pragma once

#include <algorithm>
#include <cstddef>
#include <climits>
#include <cstring>
#include <ostream>
#include <type_traits>

#include <yas/detail/type_traits/serializer.hpp>
#include <yas/detail/tools/cast.hpp>

#include <cassert>

namespace prototype::message {

template <typename CharT, typename Traits = std::char_traits<CharT>>
class nssso_string {
  using UCharT = typename std::make_unsigned<CharT>::type;

public:
  static std::size_t const sso_capacity = 62;

  nssso_string() noexcept
      : nssso_string{"", static_cast<std::size_t>(0)} {
  }
  
  nssso_string(const std::string& str)
      : nssso_string(str.data(), str.size()) {
  }
  
  nssso_string(const CharT* string, std::size_t size) {
    if (size < sso_capacity) {
      Traits::move(data_.short_.string, string, size);
      Traits::assign(data_.short_.string[size], static_cast<CharT>(0));
      set_sso_size(size);
    } else {
      data_.long_.ptr = new CharT[size + 1];
      Traits::move(data_.long_.ptr, string, size);
      Traits::assign(data_.long_.ptr[size], static_cast<CharT>(0));
      set_long_data(size, size);
    }
  }
  
  nssso_string(CharT const* string)
      : nssso_string{string, std::strlen(string)} {
  }
  
  nssso_string(const nssso_string& string) {
    if(string.sso()) {
      data_.short_ = string.data_.short_;
    } else {
      new (this) nssso_string{string.data(), string.size()};
    }
  }
  
  nssso_string(nssso_string&& string) noexcept {
    data_ = string.data_;
    string.set_moved_from();
  }
  
  void resize(std::size_t size) {
    if(size < sso_capacity) {
      set_sso_size(size);
    } else {
      this->~nssso_string();
      data_.long_.ptr = new CharT[size + 1];
      set_long_data(size, size);
    }
  }
  
  nssso_string& operator=(nssso_string other) {
    swap(other, *this);
    return *this;
  }
  
  nssso_string& operator=(nssso_string&& other) {
    this->~nssso_string();
    data_ = other.data_;
    other.set_moved_from();
    return *this;
  }
  
  ~nssso_string() {
    if(!sso()) {
      delete[] data_.long_.ptr;
    }
  }
  
  operator std::string() const {
    return {data(), size()};
  }
  
  const CharT* data() const noexcept {
    return sso() ? data_.short_.string : data_.long_.ptr;
  }
  
  std::size_t size() const noexcept {
    if(sso())
      return sso_size();
    else
      return long_data().first;
  }
  
  std::size_t capacity() const noexcept {
    if(sso())
      return sso_capacity;
    else
      return long_data().second;
  }
  
  friend void swap(nssso_string& lhs, nssso_string& rhs) {
    std::swap(lhs.data_, rhs.data_);
  }
  
private:
  void set_moved_from() {
    set_sso_size(0);
  }
  
  bool sso() const noexcept {
    return data_.short_.is_sso;
  }
  
  void set_sso_size(unsigned char size) noexcept {
    data_.short_.is_sso = 1;
    data_.short_.size = size;
  }
  
  std::size_t sso_size() const noexcept {
    return data_.short_.size;
  }
  
  void set_long_data(std::size_t size, std::size_t capacity) {
    data_.long_.is_sso = 0;
    data_.long_.size = size;
    data_.long_.capacity = capacity;
  }
  
  std::pair<std::size_t, std::size_t> long_data() const {
    auto size = data_.long_.size;
    auto capacity = data_.long_.capacity;
    return {size, capacity};
  }
  
private:
  union Data {
    struct Short {
      CharT string[sso_capacity];
      UCharT size;
      UCharT is_sso;
    } short_;
    struct Long {
      CharT* ptr;
      std::size_t size;
      std::size_t capacity;
      CharT pad[sizeof(Short) - sizeof(CharT*) - 2 * sizeof(std::size_t) - sizeof(UCharT)];
      UCharT is_sso;
    } long_;
  } data_;
 
  static_assert(sizeof(typename Data::Long) == sizeof(typename Data::Short));
  static_assert(offsetof(typename Data::Long, is_sso) == offsetof(typename Data::Short, is_sso));
};

template <typename CharT, typename Traits>
bool operator==(const nssso_string<CharT, Traits>& lhs, const CharT* rhs) noexcept {
  return !std::strcmp(lhs.data(), rhs);
}

template <typename CharT, typename Traits>
bool operator==(const CharT* lhs, const nssso_string<CharT, Traits>& rhs) noexcept {
  return rhs == lhs;
}

template <typename CharT, typename Traits>
bool operator==(const nssso_string<CharT, Traits>& lhs,
                const nssso_string<CharT, Traits>& rhs) noexcept {
  if(lhs.size() != rhs.size()) return false;
  return !std::strcmp(lhs.data(), rhs.data());
}

template <typename CharT, typename Traits>
std::ostream& operator<<(std::ostream& stream, const nssso_string<CharT, Traits>& string) {
  return stream << static_cast<const std::string>(string);
}

typedef nssso_string<char> String;

}

namespace yas::detail {
  
template<std::size_t F>
struct serializer<type_prop::not_a_fundamental
, ser_case::use_internal_serializer
, F
  , prototype::message::String> {
  template<typename Archive>
  static Archive& save(Archive& ar, const prototype::message::String &str) {
    ar.write_seq_size(str.size());
    ar.write(str.data(), str.size());
    
    return ar;
  }
  
  template<typename Archive>
  static Archive& load(Archive& ar, prototype::message::String &str) {
    const auto size = ar.read_seq_size();
    str.resize(size);
    ar.read(__YAS_CCAST(char*, str.data()), size);
    
    return ar;
  }
};
  
}
