#pragma once

namespace prototype::message {

using sequence_id = uint32_t;

enum id {
  ping = 1,
  pong,
  request,
  reply
};

}
