#pragma once

#include <cstdint>
#include "id.h"

namespace prototype::message {

struct Ping {
  static const id message_id = ping;
  
  int64_t t;
};

}
