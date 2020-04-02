#pragma once

#include "ping.h"
#include "pong.h"
#include "request.h"
#include "reply.h"

namespace prototype::message {
  
struct SequentialFactory {
  static Ping create_ping() {
    static int64_t t;
    return {++t};
  }
  
  static Request create_request(std::size_t string_length) {
    static char ch;
    static double f;
    static int64_t d;
    static int64_t e;
    static int32_t a;
    static int32_t b;
    static int8_t c;
    return {std::string(string_length, ++ch), ++f, ++d, ++e, ++a, ++b, ++c };
  }
};

struct FixedFactory {
  static Ping create_ping() {
    return {42};
  }
  
  static Request create_request(std::size_t string_length) {
    return {std::string(string_length, '*'), 3.1415, 1LL, 2LL, 3, 4, '5'};
  }
};

}
