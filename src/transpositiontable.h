#pragma once

#include "bitboard.h"
#include "move.h"
#include "position.h"
#include <vector>
#include <mutex>
#include <memory>

namespace JACEA
{
    class TranspositionTable
    {
    public:
        static const int flag_hash_exact = 0b00;
        static const int flag_hash_alpha = 0b01;
        static const int flag_hash_beta  = 0b10;
        static const int no_hash         = 10000000;

        struct TTEntry
        {
            u64 key;
            int depth;
            int flags;
            int value;
            Move best_move;
            std::unique_ptr<std::mutex> lock = std::make_unique<std::mutex>();
        };

        static inline size_t mb_to_size_of_hashtable(size_t mb) {
            return 1024 * 1024 * mb / (sizeof(TTEntry) + sizeof(std::mutex));
        }

        void clear_table();
        void resize_table_mb(int size_mb);

        int read_hash_entry(const Position &pos, const int alpha, const int beta, const int depth);

        void record_hash(const Position &pos, const int depth, int value, const int flag);
    private:
        std::vector<TTEntry> hash_table;
    };
}