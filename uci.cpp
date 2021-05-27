#include "uci.h"
#include "movegenerator.h"
#include "utility.h"
#include "search.h"
#include <iostream>

using namespace JACEA;

int JACEA::parse_move(Position &pos, const char *move_cstr)
{
    const int file_from = *move_cstr++ - 'a';
    const int rank_from = 8 - (*move_cstr++ - '0');
    const int from_square = rank_from * 8 + file_from;

    const int file_to = *move_cstr++ - 'a';
    const int rank_to = 8 - (*move_cstr++ - '0');
    const int to_square = rank_to * 8 + file_to;

    // TODO: Calling generate moves is costly. Generating the move manually once a mailbox it set up will increase performance. This isnt that important as this is run <500 times in an entire UCI game
    MoveList ml;
    generate_moves(pos, ml);
    for (int i = 0; i < ml.size; i++)
    {
        auto move = ml.moves[i].move;
        if (from_square == get_from_square(move) && to_square == get_to_square(move))
        {
            if (*move_cstr != '\0')
            {
                auto promo_piece = get_promoted_piece(move);
                if ((promo_piece == Q || promo_piece == q) && *move_cstr == 'q')
                    return move;
                if ((promo_piece == R || promo_piece == r) && *move_cstr == 'r')
                    return move;
                if ((promo_piece == B || promo_piece == b) && *move_cstr == 'b')
                    return move;
                if ((promo_piece == N || promo_piece == n) && *move_cstr == 'n')
                    return move;
            }
            else
                return move;
        }
    }
    return 0;
}

void JACEA::parse_go(Position &pos, std::string str)
{
    auto split = split_string(str, " ");
    UCISettings uci;
    uci.stop = false;
    uci.time_to_stop = -1;
    for (long long unsigned int i = 0; i < split.size(); i++)
    {
        if (split[i] == "wtime" && pos.get_side() == WHITE)
        {
            uci.time_to_stop = std::stoi(split[i + 1]) / 1000;
        }
        else if (split[i] == "btime" && pos.get_side() == BLACK)
        {
            uci.time_to_stop = std::stoi(split[i + 1]) / 1000;
        }
        else if (split[i] == "winc" && pos.get_side() == WHITE)
        {
            // uci.inc = std::stoi(split[i + 1]);
        }
        else if (split[i] == "binc" && pos.get_side() == BLACK)
        {
            // uci.inc = std::stoi(split[i + 1]);
        }
        else if (split[i] == "movetime")
        {
            // uci.moveTime = std::stoi(split[i + 1]);
        }
        else if (split[i] == "movestogo")
        {
            uci.moves_to_go = std::stoi(split[i + 1]);
        }
        else if (split[i] == "depth")
        {
            // uci.maxDepth = std::stoi(split[i + 1]);
        }
        else if (split[i] == "infinite")
        {
            // uci.infinite = true;
        }
    }
    // if (uci.maxDepth < 0) uci.maxDepth = MAX_MOVES;
    if (uci.time_to_stop < 0)
    {
        uci.time_to_stop = 10;
    }
    else if (uci.moves_to_go != 0)
    {
        int moves = std::min(pos.get_total_moves(), 10);
        float factor = 2 - moves / 10.0;
        float target = uci.time_to_stop / static_cast<float>(uci.moves_to_go);
        uci.time_to_stop = factor * target;
    }
    else
    {
        uci.time_to_stop = std::min(10.0, std::max(uci.time_to_stop / 20.0, 0.10));
    }
    std::cout << "Searching for: " << uci.time_to_stop << "s" << std::endl;
    uci.time_to_stop *= 1000;
    uci.time_to_stop += get_time_ms();
    search(pos, uci, max_game_ply);
}

void JACEA::parse_position(Position &pos, std::string str)
{
    auto split = split_string(str, " ");
    if (split[1] == "startpos")
    {
        pos.init_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        split.erase(split.begin(), split.begin() + 2);
    }
    else
    {
        // Init from fen
        pos.init_from_fen(split[2] + " " + split[3] + " " + split[4] + " " + split[5] + " " + split[6] + " " + split[7]);
        split.erase(split.begin(), split.begin() + 8);
    }
    if (!split.empty() && split[0] == "moves")
    {
        split.erase(split.begin(), split.begin() + 1);
        for (auto &move_str : split)
        {
            auto move = parse_move(pos, move_str.c_str());
            if (move == 0)
                assert(false);
            pos.make_move(move, MoveType::ALL);
        }
    }
    pos.reset_ply();
}
