#pragma once

#include "../base_types.h"
#include "position.h"
#include "resource_chunk.h"
#include "json/variant.h"
#include <vector>

namespace miningbots::json
{

  struct BotUpdate
  {
    BotId id; // todo rename to bot id
    Position position;
    Variant variant;

    Energy_t current_energy;
    JobId current_job_id;

    std::vector<ResourceChunk> cargo;
  };

} // namespace miningbots::json