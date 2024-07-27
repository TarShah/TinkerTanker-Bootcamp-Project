#pragma once

#include "../base_types.h"
#include <cstdint>
#include <string>

namespace miningbots::json
{

  struct TerrainConfig
  {
    std::string name{""};
    Microsecond move_interval{100000};
    Energy_t move_energy_per_interval{200};
  };

} // namespace miningbots::json