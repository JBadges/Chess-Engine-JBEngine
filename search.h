#pragma once

#include "position.h"

namespace JACEA
{
    const int value_infinite = 50000;
    const int value_mate = 40000;
    const int value_mate_lower = 39000;

    void search(Position &pos, int depth);
}