#pragma once

#include "position.h"
#include "uci.h"

namespace JACEA
{

    void search(Position &pos, std::vector<TTEntry> &tt, UCISettings &uci, int depth);
}