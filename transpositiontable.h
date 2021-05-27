#pragma once

#include "bitboard.h"
#include "move.h"
#include "position.h"
#include <vector>

namespace JACEA
{
    struct TTEntry
    {
        u64 key;
        int depth;
        int flags;
        int value;
        Move best_move;
    };

    const int flag_hash_exact = 0b00;
    const int flag_hash_alpha = 0b01;
    const int flag_hash_beta = 0b10;
    const int no_hash = 10000000;

    constexpr int hash_table_size = sizeof(TTEntry) * 1650 * 64; // 64mb

    void clear_table(std::vector<TTEntry> &table, const int size);

    static inline int read_hash_entry(const Position &pos, std::vector<TTEntry> &table, const int alpha, const int beta, const int depth)
    {
        TTEntry *entry = &table[pos.get_key() % hash_table_size];

        if (entry->key == pos.get_key())
        {
            if (entry->depth >= depth)
            {
                int score = entry->value;

                if (score > value_mate_lower)
                    score -= pos.get_ply();
                if (score < -value_mate_lower)
                    score += pos.get_ply();

                if (entry->flags == flag_hash_exact)
                {
                    // PV
                    return score;
                }
                if (entry->flags == flag_hash_alpha && score <= alpha)
                {
                    // Fail low
                    return alpha;
                }
                if (entry->flags == flag_hash_beta && score >= beta)
                {
                    // Fail high
                    return beta;
                }
            }
        }

        return no_hash;
    }

    static inline void record_hash(const Position &pos, std::vector<TTEntry> &table, const int depth, int value, const int flag)
    {
        TTEntry *entry = &table[pos.get_key() % hash_table_size];

        // If score is mating adjust for mate
        if (value > value_mate_lower)
            value += pos.get_ply();
        if (value < -value_mate_lower)
            value -= pos.get_ply();

        entry->key = pos.get_key();
        entry->depth = depth;
        entry->flags = flag;
        entry->value = value;
    }
}