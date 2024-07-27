#pragma once

#include <glaze/glaze.hpp>

namespace miningbots::json
{

  enum class UpdateType
  {
    kUseless,

    kGameStarted,
    kTickUpdate,
    kEndInWin,
    kEndInLoss,
    kEndInDraw,
    kError
  };

}
template <>
struct glz::meta<miningbots::json::UpdateType>
{
  using enum miningbots::json::UpdateType;
  static constexpr auto value = enumerate(kGameStarted, kTickUpdate, kEndInWin,
                                          kEndInLoss, kEndInDraw, kError, kUseless);
};