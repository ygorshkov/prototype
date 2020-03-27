#pragma once

#include <cstdint>
#include <string>
#include "id.h"

namespace prototype::message {

struct Reply {
  static const id message_id = reply;
  
  std::string id1;
  std::string status;
  int64_t b1;
  float e;
  int32_t a1;
  // int32_t extra_int;
  // int16_t extra_short;
  int8_t c1;
  int8_t d1;
};

}
