#pragma once
#include <random>
#include "bitboard.h"

namespace JACEA
{
    static std::random_device rd;
    static std::mt19937_64 mt19937_64(rd());
    static std::uniform_int_distribution<u64> dist(0, std::llround(std::pow(2, 62)));

    static inline u64 random_u64()
    {
        return dist(mt19937_64);
    }

    static inline u64 generate_magic_number()
    {
        return random_u64() & random_u64() & random_u64();
    }

    // From Wikipedia

    static unsigned int rand_32_state = 0b10110101101010101101010101010110;

    /* The state word must be initialized to non-zero */
    static inline unsigned int xorshift32()
    {
        /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
        rand_32_state ^= rand_32_state << 13;
        rand_32_state ^= rand_32_state >> 17;
        rand_32_state ^= rand_32_state << 5;
        return rand_32_state;
    }

    static inline unsigned int rand_in_range(unsigned int min, unsigned int max)
    {
        assert(max > min);
        return xorshift32() % (max - min) + min;
    }

}
