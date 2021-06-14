#pragma once

#include "position.h"
#include "jacea_nnue.hpp"

namespace JACEA
{

    // Mirrored and inverted for black so that sum all postion = positional eval for white
    extern int piece_square_table[2][12][64];

    enum : int
    {
        opening,
        endgame,
        middlegame
    };

    const int opening_phase_score = 6192;
    const int endgame_phase_score = 518;

    void init_pst();

    static inline int get_game_stage_score(const Position &pos)
    {
        return pos.get_material_white() + pos.get_material_black() - piece_to_value[K] - piece_to_value[k];
    }

    static inline int get_positional_score(const int game_stage_score, const int piece, const int index)
    {
        // Use opening tables
        if (game_stage_score > opening_phase_score)
        {
            return piece_square_table[opening][piece][index];
        }
        // Endgame tables
        else if (game_stage_score < endgame_phase_score)
        {
            return piece_square_table[endgame][piece][index];
        }
        // Lerp between
        else
        {
            const double open = static_cast<double>(opening_phase_score);
            const double end = static_cast<double>(endgame_phase_score);
            const double t = 1.0 / (end - open) * game_stage_score - (open / (end - open));
            return static_cast<int>(piece_square_table[opening][piece][index] + t * (piece_square_table[endgame][piece][index] - piece_square_table[opening][piece][index]));
        }
    }

    static inline int evaluation(const Position &pos)
    {
        int pieces[33];
        int squares[33];
        int nnue_score = evaluate_nnue(pos, pieces, squares);
        return nnue_score * std::max((100 - pos.get_fifty()), 0) / 100;
    }

}