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
        // Flag used for times when the search can be stopped early with no loss in information
        // ie. One legal move, Found mate
        bool end_early = false;
        bool stop_threads = false;
        int largest_depth = 0;
        unsigned long long table_base_hits = 0;
    };

    void parse_position(Position &pos, std::istringstream &tokenizer);

    Move parse_move(Position &pos, const char *move_cstr);

    void parse_go(Position &pos, std::vector<TTEntry> &tt, UCISettings &uci, std::string line);
}