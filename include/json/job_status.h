#pragma once

#include <glaze/glaze.hpp>

namespace miningbots::json {

enum class JobStatus {
  kNotStarted,
  kBlocked,
  kInProgress,
  kReachedInterval,
  kCompleted,
  kCancelledInsufficientEnergy,
  kCancelledPathBlocked,
  kCancelledBotMovedTooFar,
  kCancelled
};

}
template <> struct glz::meta<miningbots::json::JobStatus> {
  using enum miningbots::json::JobStatus;
  static constexpr auto value =
      enumerate(kNotStarted, kBlocked, kInProgress, kReachedInterval,
                kCompleted, kCancelledInsufficientEnergy, kCancelledPathBlocked,
                kCancelledBotMovedTooFar, kCancelled);
};