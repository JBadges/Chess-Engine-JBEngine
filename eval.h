#pragma once

#include "position.h"

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
        return pos.get_material_white() + pos.get_material_black();
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
        int positional_score = 0;
        for (Square square = 0; square < 64; square++)
        {
            const Piece piece = pos.get_piece_on_square(square);
            if (pos.get_piece_on_square(square) == None)
                continue;
            positional_score += get_positional_score(get_game_stage_score(pos), piece, square);
        }
        return (positional_score + pos.get_material_white() - pos.get_material_black()) * (pos.get_side() == WHITE ? 1 : -1);
    }

}