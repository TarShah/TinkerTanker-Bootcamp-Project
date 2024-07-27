#pragma once

#include "../base_types.h"
#include "bot_update.h"
#include "job_update.h"
#include "land_update.h"

#include "update_type.h"
#include <vector>

namespace miningbots::json
{

  struct Update
  {
    UpdateType update_type;

    PlayerId player_id;
    TimeStamp_t time_stamp;

    std::vector<BotUpdate> bot_updates{};
    std::vector<JobUpdate> job_updates{};
    std::vector<LandUpdate> land_updates{};
  };

} // namespace miningbots::json