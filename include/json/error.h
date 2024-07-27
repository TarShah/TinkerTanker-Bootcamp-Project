#pragma once

#include <glaze/glaze.hpp>

namespace miningbots::json {

enum Error {
  kSuccess,
  kReadJsonError,
  kInvalidGameId,
  kInvalidPlayerId,
  kInvalidBotId,
  kInvalidPlayerKey,
  kInvalidObserverKey,
  kBotAlreadyHasJob,
  kNotEnoughEnergy,
  kNotEnoughResources,
  kInvalidActionGameNotInProgress,
  kCannotStartGameAlreadyStarted,
  kCannotStartGameAlreadyEnded,
  kCannotJoinGameAlreadyJoined,
  kCannotJoinGameInvalidPlayerKey,
  kCannotJoinGameAlreadyEnded,
  kCannotSubscribeGameInvalidPlayerId,
  kCannotSubscribeGameInvalidPlayerKey,
  kCannotEndGameAlreadyEnded,
  kCannotStartNewJobCurrentJobStillRunning,
  kResourceIdNotFound,
  kTargetPositionNotWithinRange,
  kInvalidMoveBotAlreadyAtTargetPosition,
  kInvalidMoveBotPathBlocked,
  kCannotMoveBotWhileResourcesAreTransferring,
  kInvalidActionNotSubscribedToWebsocket,
  kInvalidActionForThisBotVariant,
  kCannotCancelJobNoJobRunning,
  kInvalidCreateMapRequest,
  kCannotCreateBotLimitReached,
  kUnknownError,
  kNumberOfErrors
};

}

template <> struct glz::meta<miningbots::json::Error> {
  using enum miningbots::json::Error;
  static constexpr auto value = enumerate(
      kSuccess, kReadJsonError, kInvalidGameId, kInvalidPlayerId, kInvalidBotId,
      kInvalidPlayerKey, kInvalidObserverKey, kBotAlreadyHasJob,
      kNotEnoughEnergy, kNotEnoughResources, kInvalidActionGameNotInProgress,
      kCannotStartGameAlreadyStarted, kCannotStartGameAlreadyEnded,
      kCannotJoinGameAlreadyJoined, kCannotJoinGameInvalidPlayerKey,
      kCannotJoinGameAlreadyEnded, kCannotSubscribeGameInvalidPlayerId,
      kCannotSubscribeGameInvalidPlayerKey, kCannotEndGameAlreadyEnded,
      kCannotStartNewJobCurrentJobStillRunning, kResourceIdNotFound,
      kTargetPositionNotWithinRange, kInvalidMoveBotAlreadyAtTargetPosition,
      kInvalidMoveBotPathBlocked, kCannotMoveBotWhileResourcesAreTransferring,
      kInvalidActionNotSubscribedToWebsocket, kInvalidActionForThisBotVariant,
      kCannotCancelJobNoJobRunning, kInvalidCreateMapRequest,
      kCannotCreateBotLimitReached, kUnknownError, kNumberOfErrors);
};