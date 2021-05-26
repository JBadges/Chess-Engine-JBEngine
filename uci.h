#pragma once
#include "position.h"

namespace JACEA
{
    struct UCISettings
    {
        u64 nodes = 0;
        bool stop = false;
        long long time_to_stop = -1;
        int moves_to_go = 0;
    };

    void parse_position(Position &pos, std::string str);

    Move parse_move(Position &pos, const char *move_cstr);

    void parse_go(Position &pos, std::string str);
}