#include "transpositiontable.h"

void JACEA::TranspositionTable::resize_table_mb(int size_mb)
{
    std::cout << "Resizing hash table to " << size_mb << "MB" << std::endl;
    std::cout << "Resizing hash table to " << mb_to_size_of_hashtable(size_mb) << " size" << std::endl;
    hash_table.resize(mb_to_size_of_hashtable(size_mb));
}

void JACEA::TranspositionTable::clear_table()
{
    for (auto& entry : hash_table)
    {
        std::lock_guard<std::mutex> guard(*entry.lock);
        entry.key = 0;
        entry.depth = 0;
        entry.flags = 0;
        entry.value = 0;
        entry.best_move = 0;
    }
}

int JACEA::TranspositionTable::read_hash_entry(const Position &pos, const int alpha, const int beta, const int depth) {
    TTEntry& entry = hash_table[pos.get_key() % hash_table.size()];

    // Locks until is destructed from being out of scope
    std::lock_guard<std::mutex> guard(*entry.lock);

    if (entry.key == pos.get_key())
    {
        if (entry.depth >= depth)
        {
            int score = entry.value;

            if (score > value_mate_lower)
                score -= pos.get_ply();
            if (score < -value_mate_lower)
                score += pos.get_ply();

            if (entry.flags == flag_hash_exact)
            {
                // PV
                return score;
            }
            if (entry.flags == flag_hash_alpha && score <= alpha)
            {
                // Fail low
                return alpha;
            }
            if (entry.flags == flag_hash_beta && score >= beta)
            {
                // Fail high
                return beta;
            }
        }
    }

    return no_hash;
}

void JACEA::TranspositionTable::record_hash(const Position &pos, const int depth, int value, const int flag)
{
    TTEntry& entry = hash_table[pos.get_key() % hash_table.size()];

    // Locks until is destructed from being out of scope
    std::lock_guard<std::mutex> guard(*entry.lock);

    // If score is mating adjust for mate
    if (value > value_mate_lower)
        value += pos.get_ply();
    if (value < -value_mate_lower)
        value -= pos.get_ply();

    entry.key = pos.get_key();
    entry.depth = depth;
    entry.flags = flag;
    entry.value = value;
}