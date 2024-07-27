#pragma once

#include "position.h"

namespace miningbots::json {

struct CreateMapRequest {
  Kilometer map_max_x{10};
  Kilometer map_max_y{10};
  std::vector<json::Position> start_positions{};
  // ' ' indicates empty land, 'X' indicates land with resources,
  std::vector<std::string> map{};
};

} // namespace miningbots::json