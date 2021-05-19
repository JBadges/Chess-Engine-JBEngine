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
}
