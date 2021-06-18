#include "uci.h"
#include "movegenerator.h"
#include "utility.h"
#include "search.h"
#include <sstream>
#include <iostream>
#include <string>

using namespace JACEA;

int JACEA::parse_move(Position &pos, const char *move_cstr)
{
    const int file_from = *move_cstr++ - 'a';
    const int rank_from = 8 - (*move_cstr++ - '0');
    const int from_square = rank_from * 8 + file_from;

    const int file_to = *move_cstr++ - 'a';
    const int rank_to = 8 - (*move_cstr++ - '0');
    const int to_square = rank_to * 8 + file_to;

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
                if ((promo_piece == Q || promo_piece == q) && (*move_cstr == 'q' || *move_cstr == 'Q'))
                    return move;
                if ((promo_piece == R || promo_piece == r) && (*move_cstr == 'r' || *move_cstr == 'R'))
                    return move;
                if ((promo_piece == B || promo_piece == b) && (*move_cstr == 'b' || *move_cstr == 'B'))
                    return move;
                if ((promo_piece == N || promo_piece == n) && (*move_cstr == 'n' || *move_cstr == 'N'))
                    return move;
            }
            else
                return move;
        }
    }
    return 0;
}

void JACEA::parse_go(Position &pos, std::vector<TTEntry> &tt, UCISettings &uci, std::string line)
{
    std::istringstream tokenizer{line};
    int max_depth = -1;
    int increment = 0;

    uci.stop = false;
    uci.time_to_stop = -1;

    std::string token;
    while (tokenizer >> token)
    {
        if (token == "wtime" && pos.get_side() == WHITE)
        {
            tokenizer >> token;
            uci.time_to_stop = std::stoi(token);
        }
        else if (token == "btime" && pos.get_side() == BLACK)
        {
            tokenizer >> token;
            uci.time_to_stop = std::stoi(token);
        }
        else if (token == "winc" && pos.get_side() == WHITE)
        {
            tokenizer >> token;
            increment = std::stoi(token);
        }
        else if (token == "binc" && pos.get_side() == BLACK)
        {
            tokenizer >> token;
            increment = std::stoi(token);
        }
        else if (token == "movetime")
        {
            // uci.moveTime = std::stoi(split[i + 1]);
        }
        else if (token == "movestogo")
        {
            // uci.moves_to_go = std::stoi(split[i + 1]);
        }
        else if (token == "depth")
        {
            tokenizer >> token;
            max_depth = std::stoi(token);
        }
        else if (token == "infinite")
        {
            // uci.infinite = true;
        }
    }

    if (max_depth < 0)
        max_depth = max_game_depth;
    // If time is -1 we did not recieve a time paramter so lets search for 10s
    if (uci.time_to_stop < 0)
    {
        uci.time_to_stop = 10 * 1000.0;
    }
    else
    {
        // Save 100ms of our increment each turn to protect agaisnt
        uci.time_to_stop += std::max(increment - 100, 0);
        // Save another 100ms as margin to return our move
        uci.time_to_stop -= 100;
        if (uci.time_to_stop < 0)
        {
            uci.time_to_stop = 0;
        }
        else
        {
            uci.time_to_stop = std::min(10 * 1000.0, uci.time_to_stop / 20.0);
        }
    }
    std::cout << "Searching for: " << uci.time_to_stop << "ms"
              << " to a max depth of " << max_depth << std::endl;
    uci.time_to_stop += get_time_ms();
    search(pos, tt, uci, max_depth);
}

void JACEA::parse_position(Position &pos, std::istringstream &tokenizer)
{
    std::string token;
    tokenizer >> token;

    if (token == "startpos")
    {
        pos.init_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        tokenizer >> token; // Since fen option removes the moves token
    }
    else if (token == "fen")
    {
        // Init from fen
        std::string fen = "";
        while (tokenizer >> token && token != "moves")
            fen += token + " ";
        pos.init_from_fen(fen);
    }
    else
    {
        // We should have had either startpos or fen
        assert(false);
        return;
    }
    while (tokenizer >> token)
    {
        auto move = parse_move(pos, token.c_str());
        if (move == 0)
            assert(false);
        pos.make_move(move, MoveType::ALL);
    }
    pos.reset_ply();
}
