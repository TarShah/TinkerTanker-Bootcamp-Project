#pragma once
#include <glaze/glaze.hpp>

namespace miningbots::json
{

  enum class Target
  {
    kNoTarget,
    kLand,
    kBot,
    kResource
  };

}

template <>
struct glz::meta<miningbots::json::Target>
{
  using enum miningbots::json::Target;
  static constexpr auto value = enumerate(kNoTarget, kLand, kBot, kResource);
};