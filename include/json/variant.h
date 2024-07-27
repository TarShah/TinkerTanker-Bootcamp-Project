#pragma once

#include <glaze/glaze.hpp>

namespace miningbots::json
{

  enum class Variant
  {
    kFactoryBot,
    kMiningBot
  };

}

template <>
struct glz::meta<miningbots::json::Variant>
{
  using enum miningbots::json::Variant;
  static constexpr auto value = enumerate(kFactoryBot, kMiningBot);
};