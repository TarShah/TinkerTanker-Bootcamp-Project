#pragma once

#include <glaze/glaze.hpp>

namespace miningbots::json
{

  enum class GameStatus
  {
    kNotStarted,
    kOpen,
    kFull,
    kEnded
  };

}
template <>
struct glz::meta<miningbots::json::GameStatus>
{
  using enum miningbots::json::GameStatus;
  static constexpr auto value = enumerate(kNotStarted, kOpen, kFull, kEnded);
};