#pragma once
#include "position.h"
#include "transpositiontable.h"
#include <sstream>

namespace JACEA
{
    struct UCISettings
    {
        u64 nodes = 0;
        bool stop = false;
        long long time_to_stop = -1;
        int moves_to_go = 0;
        bool completed_iteration = false;
        bool stop_threads = false;
        int largest_depth = 0;
        unsigned long long table_base_hits = 0;
    };

    void parse_setoption(TranspositionTable &tt, std::istringstream &tokenizer);

    void parse_position(Position &pos, std::istringstream &tokenizer);

    Move parse_move(Position &pos, const char *move_cstr);

    void parse_go(Position &pos, TranspositionTable &tt, UCISettings &uci, std::istringstream& tokenizer);
}