#pragma once

#include "position.h"

namespace JACEA
{

    static inline int evaluation(const Position &pos)
    {
        return (pos.get_material_white() - pos.get_material_black()) * (pos.get_side() == WHITE ? 1 : -1);
    }

}