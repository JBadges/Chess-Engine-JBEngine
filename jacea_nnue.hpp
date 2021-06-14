#pragma once

#include "position.h"
#include "nnue/nnue.h"

namespace JACEA
{
    static inline void nnue_input(const Position &pos, int *out_piece, int *out_squares)
    {
        int i = 2;
        for (Square sq = 0; sq < 64; sq++)
        {
            Piece piece = pos.get_piece_on_square(sq);
            if (piece == None)
                continue;

            if (piece == K)
            {
                out_piece[0] = to_nnue_piece[piece];
                out_squares[0] = to_nnue_square[sq];
            }
            else if (piece == k)
            {
                out_piece[1] = to_nnue_piece[piece];
                out_squares[1] = to_nnue_square[sq];
            }
            else
            {
                out_piece[i] = to_nnue_piece[piece];
                out_squares[i] = to_nnue_square[sq];
                i++;
            }
        }
        out_piece[i] = 0;
        out_squares[i] = 0;
    }

    static inline int evaluate_nnue(const Position &pos, int *pieces, int *squares)
    {
        nnue_input(pos, pieces, squares);
        int score = nnue_evaluate(pos.get_side(), pieces, squares);
        return score;
    }
}