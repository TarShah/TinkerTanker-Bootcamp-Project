// DONE
#pragma once
#include <chrono>
#include <cstdint>
#include <string>

using Second = int16_t;
using MilliSecond = uint32_t;
using Microsecond = uint64_t;
using TimeStamp_t = Microsecond; // these are just unsigned int type objects

using Id = uint32_t;
using ResourceId = uint16_t;
using TerrainId = uint16_t; // Id's are unsigned int type objects

using PlayerId = Id;
using GameId = Id;
using BotId = Id;
using JobId = Id;
using ActionId = uint16_t;
using VariantId = uint8_t;
using ConfigId = uint16_t; // Id's are unsigned int type objects

using Key_t = Id;
using Rarity_t = float;
using Energy_t = int16_t;
using Kilometer = uint16_t;
using Kilogram = uint16_t; // wieghts and energy nd all just unsigned type objects

const std::string kConfigPath = "../config/";
const size_t kMinPlayersToStartGame = 2;
const size_t kMaxPlayers = 4;

const Energy_t kMaxEnergy = 1000;
const Microsecond kEnergyRechargeInterval = 1024768;
const Energy_t kEnergyRechargedPerInterval = 50;

const MilliSecond kTickInterval = 32;

[[maybe_unused]]
// returns current time using std::chrono
static inline TimeStamp_t TimeNow()
{
  return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

[[maybe_unused]]
// The function converts a Microsecond value into a std::chrono::microseconds duration object.
// This is useful for working with time durations in a type-safe manner using the C++ std::chrono library.
static inline std::chrono::microseconds DurationFromMicros(Microsecond micros)
{
  return std::chrono::microseconds(micros);
}

template <typename T>
[[maybe_unused]]
// returns the size of any variable, but the function name is Index somehow ??
// size_t is the type for representing the sizes of variables in C++(consider it as special case of unsigned int type)
static inline constexpr size_t Index(T id)
{
  return static_cast<size_t>(id);
}
