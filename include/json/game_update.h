#pragma once

#include "../base_types.h"
#include "game_status.h"
#include <cstdint>
#include <string>

namespace miningbots::json
{

  struct GameUpdate
  {
    GameId game_id;
    GameStatus game_status;
    uint16_t current_players;
    uint16_t max_players;
    std::string game_name;
  };

} // namespace miningbots::json