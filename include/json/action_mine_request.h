#pragma once

#include "position.h"
#include "json/resource_chunk.h"

namespace miningbots::json
{

  struct ActionMineRequest
  {
    GameId game_id;
    PlayerId player_id;
    Key_t player_key;
    BotId bot_id;
    Position target_position;
    ResourceChunk target_chunk;
  };

} // namespace miningbots::json
