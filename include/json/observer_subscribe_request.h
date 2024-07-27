#pragma once

#include "../base_types.h"

namespace miningbots::json
{

  struct ObserverSubscribeRequest
  {
    GameId game_id;
    Key_t observer_key;
    std::string observer_name;
  };

} // namespace miningbots::json
