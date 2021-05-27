#include "search.h"
#include "eval.h"
#include "movegenerator.h"
#include "utility.h"
#include <iostream>
#include <algorithm>

using namespace JACEA;

int best_move = -1;

static inline bool compareDescendingMoves(const ScoredMove &a, const ScoredMove &b)
{
    return a.score > b.score;
}

static inline void update_stop(UCISettings &uci)
{
    uci.stop = get_time_ms() > uci.time_to_stop;
}

static inline int quiesence(Position &pos, int alpha, int beta, UCISettings &uci)
{
    uci.nodes++;

    if (uci.nodes & 15000) // ~ each ms
    {
        update_stop(uci);
    }
    if (uci.stop)
    {
        return 0;
    }
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
    std::sort(ml.moves, ml.moves + ml.size, compareDescendingMoves);
    for (int i = 0; i < ml.size; i++)
    {
        if (!pos.make_move(ml.moves[i].move, MoveType::CAPTURES))
            continue;

        const int score = -quiesence(pos, -beta, -alpha, uci);

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

static inline int negamax(Position &pos, int alpha, int beta, int depth, UCISettings &uci)
{
    uci.nodes++;

    if (uci.nodes & 15000) // ~ each ms
    {
        update_stop(uci);
    }

    if (uci.stop)
    {
        return 0;
    }

    // Draw
    if (pos.get_ply() != 0 && pos.three_fold_repetition())
    {
        return 0;
    }

    int score;

    if (depth == 0)
        return quiesence(pos, alpha, beta, uci);

    if (pos.get_ply() >= max_game_ply)
        return evaluation(pos);

    bool in_check = pos.is_square_attacked(pos.get_side() ^ 1, (pos.get_side() == WHITE) ? get_firstlsb_index(pos.get_piece_board(K)) : get_firstlsb_index(pos.get_piece_board(k)));

    if (in_check)
        depth++;

    int legal_moves = 0;

    MoveList ml;
    generate_moves(pos, ml);
    std::sort(ml.moves, ml.moves + ml.size, compareDescendingMoves);
    for (int i = 0; i < ml.size; i++)
    {
        if (!pos.make_move(ml.moves[i].move, MoveType::ALL))
            continue;

        legal_moves++;

        score = -negamax(pos, -beta, -alpha, depth - 1, uci);
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

void JACEA::search(Position &pos, UCISettings &uci, int depth)
{
    int real_best = best_move;

    for (int current_depth = 1; current_depth <= depth; current_depth++)
    {
        std::cout << "Starting search for depth: " << current_depth << std::endl;
        int score = negamax(pos, -value_infinite, value_infinite, current_depth, uci);
        if (!uci.stop)
        {
            real_best = best_move;
        }
        else
        {
            std::cout << "Timed out of search: " << current_depth << std::endl;
            break;
        }
        if (score > -value_mate && score < -value_mate_lower)
        {
            printf("info score mate %d depth %d nodes %llu pv ", -(score + value_mate) / 2 - 1, current_depth, uci.nodes);
        }
        else if (score > value_mate_lower && score < value_mate)
        {
            printf("info score mate %d depth %d nodes %llu pv ", (value_mate - score) / 2 + 1, current_depth, uci.nodes);
        }
        else
        {
            printf("info score cp %d depth %d nodes %llu pv ", score, current_depth, uci.nodes);
        }
        std::cout << square_to_coordinate[get_from_square(real_best)] << square_to_coordinate[get_to_square(real_best)];
        if (get_promoted_piece(real_best) != 0)
            std::cout << piece_to_string[get_promoted_piece(real_best)];
        std::cout << std::endl;
    }
    std::cout << "bestmove " << square_to_coordinate[get_from_square(real_best)] << square_to_coordinate[get_to_square(real_best)];
    if (get_promoted_piece(real_best) != 0)
        std::cout << piece_to_string[get_promoted_piece(real_best)];
    std::cout << std::endl;
}