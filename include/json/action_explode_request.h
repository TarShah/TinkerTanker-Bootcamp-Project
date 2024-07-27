#pragma once

#include "position.h"

namespace miningbots::json
{

  struct ActionExplodeRequest
  {
    GameId game_id;
    PlayerId player_id;
    Key_t player_key;
    BotId bot_id;
  };

} // namespace miningbots::json
