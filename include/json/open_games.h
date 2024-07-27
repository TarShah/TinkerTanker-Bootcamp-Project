#pragma once

#include "../base_types.h"
#include "game.h"
#include <vector>

namespace miningbots::json
{

  struct OpenGames
  {
    std::vector<Game> games;
  };

} // namespace miningbots::json