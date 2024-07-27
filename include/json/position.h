#pragma once

#include "../base_types.h"
#include <cstdint>

namespace miningbots::json
{

  struct Position
  {
    Kilometer x;
    Kilometer y;
    bool operator==(const Position &other) const
    {
      return (x == other.x) && (y == other.y);
    }
    bool operator!=(const Position &other) const
    {
      return (x != other.x) || (y != other.y);
    }
    bool operator<(const Position &other) const
    {
      return std::make_pair(x, y) < std::make_pair(other.x, other.y);
    }
    bool operator>(const Position &other) const
    {
      return std::make_pair(x, y) > std::make_pair(other.x, other.y);
    }
    int square_distance(const Position &other) const
    {
      return ((other.x - x) * (other.x - x) + (other.y - y) * (other.y - y));
    }
    int square() const
    {
      return (x * x + y * y);
    }
  };

} // namespace miningbots::json

template <>
struct std::hash<miningbots::json::Position>
{
  std::size_t operator()(const miningbots::json::Position &key) const
  {
    return hash<int32_t>()((key.x << 14) + key.y);
  }
};
