#pragma once

#include "../base_types.h"

namespace miningbots::json {

struct ServerConfig {
  std::string hostname;
  int port;
};

} // namespace miningbots::json