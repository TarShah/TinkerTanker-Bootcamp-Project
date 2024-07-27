#pragma once

#include "../base_types.h"

namespace miningbots::json
{

  struct ResourceChunk
  {
    ResourceId id;
    Kilogram amount;
  };

} // namespace miningbots::json