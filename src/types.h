#pragma once
#include <string>

namespace JACEA
{
    constexpr int max_game_ply = 500;
    constexpr int max_game_depth = 64;
    constexpr int max_moves = 250;

    typedef int Color;

    enum eColor : int
    {
        WHITE,
        BLACK,
        BOTH
    };

    enum Castling : int
    {
        wk = 0b0001,
        wq = 0b0010,
        bk = 0b0100,
        bq = 0b1000
    };

    typedef int Piece;
    enum ePiece : int
    {
        P,
        N,
        B,
        R,
        Q,
        K,
        p,
        n,
        b,
        r,
        q,
        k,
        None
    };

    const Piece to_nnue_piece[12] = {6, 5, 4, 3, 2, 1, 12, 11, 10, 9, 8, 7};

    const Color color_from_piece[] = {
        WHITE,
        WHITE,
        WHITE,
        WHITE,
        WHITE,
        WHITE,
        BLACK,
        BLACK,
        BLACK,
        BLACK,
        BLACK,
        BLACK,
        None};

    const std::string piece_to_string[13] = {"P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k", "-"};

    inline Piece char_to_piece(const char c)
    {
        switch (c)
        {
        case 'P':
            return P;
        case 'N':
            return N;
        case 'B':
            return B;
        case 'R':
            return R;
        case 'Q':
            return Q;
        case 'K':
            return K;
        case 'p':
            return p;
        case 'n':
            return n;
        case 'b':
            return b;
        case 'r':
            return r;
        case 'q':
            return q;
        case 'k':
            return k;
        default:
            assert(false);
        }
        return None;
    }

    typedef int Move;

    /**
     * Evaluation
     */
    const int piece_to_value[] = {
        100,
        320,
        330,
        500,
        900,
        12000,
        100,
        320,
        330,
        500,
        900,
        12000};
    struct ScoredMove
    {
        Move move;
        int score = 0;
    };

    struct MoveList
    {
        ScoredMove moves[max_moves];
        int size = 0;
    };

    const int value_infinite = 50000;
    const int value_mate = 40000;
    const int value_mate_lower = 39000;
    const int value_win = 899;
}