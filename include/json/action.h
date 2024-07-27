#pragma once

#include <glaze/glaze.hpp>

namespace miningbots::json
{

  enum Action
  {
    kNoAction,
    kMove,
    kScan,
    kMine,
    kTransfer,
    kBuildBot,
    kExplode,
    kNumberOfActions
  };

}

template <>
struct glz::meta<miningbots::json::Action>
{
  using enum miningbots::json::Action;
  static constexpr auto value =
      enumerate(kNoAction, kMove, kScan, kMine, kTransfer, kBuildBot, kExplode,
                kNumberOfActions);
};