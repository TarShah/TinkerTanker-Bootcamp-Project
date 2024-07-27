#pragma once

#include "../base_types.h"
#include <cstdint>
#include <string>

namespace miningbots::json
{

  struct ResourceConfig
  {
    std::string name{""};
    Microsecond mine_interval{100000};
    Energy_t mine_energy_per_interval{200};
    uint16_t mine_amount_per_interval{20};
    Rarity_t rarity{0.0};
  };

} // namespace miningbots::json