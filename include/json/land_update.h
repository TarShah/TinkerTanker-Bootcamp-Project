#pragma once

#include "base_types.h"
#include "position.h"
#include "json/resource_chunk.h"

#include <vector>
namespace miningbots::json
{

  struct LandUpdate
  {
    Position position;
    TerrainId terrain_id;
    bool is_traversable;
    std::vector<ResourceChunk> resources;
  };

} // namespace miningbots::json