#pragma once

#include "../base_types.h"

#include <string>

namespace miningbots::json
{

  struct JoinGameRequest
  {
    GameId game_id;
    std::string player_name;
    Key_t player_key;
  };

} // namespace miningbots::json