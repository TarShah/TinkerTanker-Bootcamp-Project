#pragma once

#include "position.h"
#include "resource_chunk.h"

namespace miningbots::json
{

  struct ActionCancelJobRequest
  {
    GameId game_id;
    PlayerId player_id;
    Key_t player_key;
    BotId bot_id; // the job of this bot will be cancelled
  };

} // namespace miningbots::json
