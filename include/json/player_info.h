#pragma once

#include "../base_types.h"

#include <string>

namespace miningbots::json
{

  struct PlayerInfo
  {
    PlayerId session_id{0};
    std::string name{"unassigned"};
    bool is_in_game{false};
  };

} // namespace miningbots::json