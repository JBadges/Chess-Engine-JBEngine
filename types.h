#pragma once
#include <string>

namespace JACEA
{
    constexpr int max_game_ply = 2 << 8;

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
        ScoredMove moves[max_game_ply];
        int size = 0;
    };

    const int value_infinite = 50000;
    const int value_mate = 40000;
    const int value_mate_lower = 39000;
}