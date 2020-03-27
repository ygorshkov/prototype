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
  
  static Request create_request() {
    static char ch;
    static double f;
    static int64_t d;
    static int64_t e;
    static int32_t a;
    static int32_t b;
    static int8_t c;
    return {std::string(50, ++ch), ++f, ++d, ++e, ++a, ++b, ++c };
  }
};

/// Small String Optimization Factory
struct SSOFactory {
  static Ping create_ping() {
    return {42};
  }
  
  static Request create_request() {
    return {std::string(std::string().capacity(), '*'), 3.1415, 1LL, 2LL, 3, 4, '5'};
  }
};

/// Not So Small String Factory
struct NSSSFactory {
  static Ping create_ping() {
    return {42};
  }
  
  static Request create_request() {
    return {std::string(std::string().capacity() + 1, '*'), 3.1415, 1LL, 2LL, 3, 4, '5'};
  }
};
  
/// Giant String Factory
struct GiantFactory {
  static Ping create_ping() {
    return {42};
  }
  
  static Request create_request() {
    return {std::string(100'000, '*'), 3.1415, 1LL, 2LL, 3, 4, '5'};
  }
};

}
