#include "search.h"
#include "eval.h"
#include "movegenerator.h"
#include <iostream>

using namespace JACEA;

int best_move = -1;

static inline int quiesence(Position &pos, int alpha, int beta)
{
    if (pos.get_ply() >= max_game_ply)
        return evaluation(pos);

    int eval = evaluation(pos);

    if (eval >= beta)
        return beta;

    if (eval > alpha)
    {
        alpha = eval;
    }

    MoveList ml;
    generate_moves(pos, ml);
    for (int i = 0; i < ml.size; i++)
    {
        if (!pos.make_move(ml.moves[i].move, MoveType::CAPTURES))
            continue;

        const int score = -quiesence(pos, -beta, -alpha);

        pos.take_move();

        // PV move found
        if (score > alpha)
        {
            alpha = score;

            // Fail-hard (failed high)
            if (score >= beta)
                return beta;
        }
    }

    // failed low
    return alpha;
}

static inline int negamax(Position &pos, int alpha, int beta, int depth)
{
    // Draw
    if (pos.get_ply() != 0 && pos.three_fold_repetition())
    {
        return 0;
    }

    int score;

    if (depth == 0)
        return quiesence(pos, alpha, beta);

    if (pos.get_ply() >= max_game_ply)
        return evaluation(pos);

    bool in_check = pos.is_square_attacked(pos.get_side() ^ 1, (pos.get_side() == WHITE) ? get_firstlsb_index(pos.get_piece_board(K)) : get_firstlsb_index(pos.get_piece_board(k)));

    if (in_check)
        depth++;

    int legal_moves = 0;

    MoveList ml;
    generate_moves(pos, ml);

    for (int i = 0; i < ml.size; i++)
    {
        if (!pos.make_move(ml.moves[i].move, MoveType::ALL))
            continue;

        legal_moves++;

        score = -negamax(pos, -beta, -alpha, depth - 1);
        pos.take_move();

        // PV move found
        if (score > alpha)
        {
            alpha = score;
            if (pos.get_ply() == 0)
            {
                best_move = ml.moves[i].move;
            }

            // Fail-hard (failed high)
            if (score >= beta)
            {
                return beta;
            }
        }
    }

    if (legal_moves == 0)
    {
        if (in_check)
        {
            return -value_mate + pos.get_ply();
        }
        return 0;
    }

    // failed low
    return alpha;
}

void JACEA::search(Position &pos, int depth)
{
    for (int current_depth = 1; current_depth <= depth; current_depth++)
    {
        negamax(pos, -value_infinite, value_infinite, current_depth);
        std::cout << "Best move at depth " << current_depth << ": " << square_to_coordinate[get_from_square(best_move)] << square_to_coordinate[get_to_square(best_move)] << std::endl;
    }
}