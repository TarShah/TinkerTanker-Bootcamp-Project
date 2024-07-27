#pragma once
#include <glaze/glaze.hpp>

namespace miningbots::json
{

  enum Terrain
  {
    kUnassigned,
    kGrasslands,
    kHills,
    kMountains,
    kRivers,
    kOcean,
    kNumberOfTerrains
  };

}

template <>
struct glz::meta<miningbots::json::Terrain>
{
  using enum miningbots::json::Terrain;
  static constexpr auto value =
      enumerate(kUnassigned, kGrasslands, kHills, kMountains, kRivers, kOcean,
                kNumberOfTerrains);
};