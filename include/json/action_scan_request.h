#pragma once

#include "position.h"

namespace miningbots::json
{

  struct ActionScanRequest
  {
    GameId game_id;
    PlayerId player_id;
    Key_t player_key;
    BotId bot_id;
    Position target_position;
  };

} // namespace miningbots::json
