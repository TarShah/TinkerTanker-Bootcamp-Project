#pragma once

#include "../base_types.h"
#include "json/resource_chunk.h"
#include "json/resource_config.h"
#include "json/terrain_config.h"

namespace miningbots::json
{

  struct MapConfig
  {
    Kilometer max_x{10};
    Kilometer max_y{10};

    size_t min_players_to_start_game{1};
    size_t max_players{4};

    Energy_t max_bot_energy{1000};
    Microsecond energy_recharge_interval{640000};
    Energy_t energy_recharge_per_interval{200};

    Microsecond action_scan_interval{64000};
    Energy_t action_scan_energy{400};
    Kilometer action_scan_radius{1};
    Kilometer action_scan_max_distance{5};

    Kilometer action_mine_max_distance{1};

    Microsecond action_transfer_interval{128000};
    Energy_t action_transfer_energy_per_interval{100};
    Kilogram action_transfer_amount_per_interval{60};
    Kilometer action_transfer_max_distance{1};

    Microsecond action_buildbot_interval{2048000};
    Energy_t action_buildbot_energy{1000};

    Microsecond action_explode_interval{32000};
    Energy_t action_explode_minimum_energy{200};
    Energy_t action_explode_energy_per_radius{300};

    // const MilliSecond kTickInterval = 784;
    // const MilliSecond kTickInterval = 1024;
    MilliSecond tick_interval{32};

    std::vector<TerrainConfig> terrain_configs{{}};
    std::vector<ResourceConfig> resource_configs{{}};
    std::vector<ResourceChunk> win_condition{};
    std::vector<json::ResourceChunk> action_buildbot_resource_cost = {{1, 50}};
  };

} // namespace miningbots::json