#pragma once

#include <cstdint>
#include <string>
#include "nssso_string.h"
#include "id.h"

namespace prototype::message {

struct Request {
  static const id message_id = request;

  String id;
  double f;
  int64_t d;
  int64_t e;
  int32_t a;
  int32_t b;
  // int32_t extra_int;
  // int16_t extra_short;
  // int8_t extra_byte;
  int8_t c;
};

}
