#pragma once

#include <cstdint>
#include "id.h"

namespace prototype::message {

struct Pong {
  static const id message_id = pong;
  
  int64_t t1;
  int64_t t2;
  // int32_t extra_int;
  // int16_t extra_short;
  // int8_t extra_byte;
  int8_t status;
};

}
