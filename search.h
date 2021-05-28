#pragma once

#include "position.h"
#include "uci.h"

namespace JACEA
{
    inline constexpr int mate_in(int ply)
    {
        return value_mate - ply;
    }

    inline constexpr int mated_in(int ply)
    {
        return -value_mate + ply;
    }

    void search(Position &pos, std::vector<TTEntry> &tt, UCISettings &uci, int depth);
}