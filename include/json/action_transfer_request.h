#pragma once

#include "position.h"
#include "resource_chunk.h"

namespace miningbots::json
{

  struct ActionTransferRequest
  {
    GameId game_id;
    PlayerId player_id;
    Key_t player_key;
    BotId bot_id;
    Position target_position;
    Id target_id; // this is the bot id you are transferring to
    ResourceChunk target_chunk;
  };

} // namespace miningbots::json
