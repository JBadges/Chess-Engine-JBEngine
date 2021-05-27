#pragma once

#include "position.h"

namespace JACEA
{

    // Mirrored and inverted for black so that sum all postion = positional eval for white
    extern int piece_square_table[12][64];

    void init_pst();

    static inline int evaluation(const Position &pos)
    {
        int positional_score = 0;
        for (Square square = 0; square < 64; square++)
        {
            const Piece piece = pos.get_piece_on_square(square);
            if (pos.get_piece_on_square(square) == None)
                continue;
            positional_score += piece_square_table[piece][square];
        }
        return (positional_score + pos.get_material_white() - pos.get_material_black()) * (pos.get_side() == WHITE ? 1 : -1);
    }

}