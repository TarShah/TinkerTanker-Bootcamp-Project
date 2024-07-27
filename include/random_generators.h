// DONE
#pragma once
#include <cstdint>
#include <random>

namespace
{

  std::random_device rd;
  std::mt19937_64 generator(rd());
  std::uniform_int_distribution<uint32_t> rand_uint32_distribution;
  std::uniform_int_distribution<int16_t> rand_uint16_distribution;

}

/**
 * Generates a random 32-bit unsigned integer. Used mostly to generate ids,
 * Cannot return 0, as ids use that as a special case to indicate error.
 *
 * @return The generated random number.
 */
[[maybe_unused]]
static uint32_t RandomUInt_32()
{
  uint32_t random_number = rand_uint32_distribution(generator);
  while (random_number == 0)
  {
    random_number = rand_uint32_distribution(generator);
  }
  return random_number;
}

/**
 * Generates a random unsigned 16-bit integer. Mostly used for Position x and y
 *
 * @return The randomly generated unsigned 16-bit integer.
 */
[[maybe_unused]]
static uint16_t RandomUInt_16()
{
  return rand_uint16_distribution(generator);
}
