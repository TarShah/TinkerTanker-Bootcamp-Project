#pragma once

#include "../base_types.h"

namespace miningbots::json
{

  struct ClientSubscribeRequest
  {
    GameId game_id;
    Key_t player_key;
    PlayerId player_id;
  };

} // namespace miningbots::json
