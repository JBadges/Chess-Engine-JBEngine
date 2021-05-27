#include "eval.h"

using namespace JACEA;

int JACEA::piece_square_table[12][64] = {0};

void JACEA::init_pst()
{
    const int pawn_st[64] = {0, 0, 0, 0, 0, 0, 0, 0,
                             50, 50, 50, 50, 50, 50, 50, 50,
                             10, 10, 20, 30, 30, 20, 10, 10,
                             5, 5, 10, 25, 25, 10, 5, 5,
                             0, 0, 0, 20, 20, 0, 0, 0,
                             5, -5, -10, 0, 0, -10, -5, 5,
                             5, 10, 10, -20, -20, 10, 10, 5,
                             0, 0, 0, 0, 0, 0, 0, 0};
    const int knight_st[64] = {
        -50,
        -40,
        -30,
        -30,
        -30,
        -30,
        -40,
        -50,
        -40,
        -20,
        0,
        0,
        0,
        0,
        -20,
        -40,
        -30,
        0,
        10,
        15,
        15,
        10,
        0,
        -30,
        -30,
        5,
        15,
        20,
        20,
        15,
        5,
        -30,
        -30,
        0,
        15,
        20,
        20,
        15,
        0,
        -30,
        -30,
        5,
        10,
        15,
        15,
        10,
        5,
        -30,
        -40,
        -20,
        0,
        5,
        5,
        0,
        -20,
        -40,
        -50,
        -40,
        -30,
        -30,
        -30,
        -30,
        -40,
        -50,
    };
    const int bishop_st[64] = {
        -20,
        -10,
        -10,
        -10,
        -10,
        -10,
        -10,
        -20,
        -10,
        0,
        0,
        0,
        0,
        0,
        0,
        -10,
        -10,
        0,
        5,
        10,
        10,
        5,
        0,
        -10,
        -10,
        5,
        5,
        10,
        10,
        5,
        5,
        -10,
        -10,
        0,
        10,
        10,
        10,
        10,
        0,
        -10,
        -10,
        10,
        10,
        10,
        10,
        10,
        10,
        -10,
        -10,
        5,
        0,
        0,
        0,
        0,
        5,
        -10,
        -20,
        -10,
        -10,
        -10,
        -10,
        -10,
        -10,
        -20,
    };
    const int rook_st[64] = {0, 0, 0, 0, 0, 0, 0, 0,
                             5, 10, 10, 10, 10, 10, 10, 5,
                             -5, 0, 0, 0, 0, 0, 0, -5,
                             -5, 0, 0, 0, 0, 0, 0, -5,
                             -5, 0, 0, 0, 0, 0, 0, -5,
                             -5, 0, 0, 0, 0, 0, 0, -5,
                             -5, 0, 0, 0, 0, 0, 0, -5,
                             0, 0, 0, 5, 5, 0, 0, 0};
    const int queen_st[64] = {-20, -10, -10, -5, -5, -10, -10, -20,
                              -10, 0, 0, 0, 0, 0, 0, -10,
                              -10, 0, 5, 5, 5, 5, 0, -10,
                              -5, 0, 5, 5, 5, 5, 0, -5,
                              0, 0, 5, 5, 5, 5, 0, -5,
                              -10, 5, 5, 5, 5, 5, 0, -10,
                              -10, 0, 5, 0, 0, 0, 0, -10,
                              -20, -10, -10, -5, -5, -10, -10, -20};
    const int king_st[64] = {-30, -40, -40, -50, -50, -40, -40, -30,
                             -30, -40, -40, -50, -50, -40, -40, -30,
                             -30, -40, -40, -50, -50, -40, -40, -30,
                             -30, -40, -40, -50, -50, -40, -40, -30,
                             -20, -30, -30, -40, -40, -30, -30, -20,
                             -10, -20, -20, -20, -20, -20, -20, -10,
                             20, 20, 0, 0, 0, 0, 20, 20,
                             20, 30, 10, 0, 0, 10, 30, 20};
    for (Square square = 0; square < 64; square++)
    {
        piece_square_table[P][square] = pawn_st[square];
        piece_square_table[p][square] = -pawn_st[square ^ 56];

        piece_square_table[N][square] = knight_st[square];
        piece_square_table[n][square] = -knight_st[square ^ 56];

        piece_square_table[B][square] = bishop_st[square];
        piece_square_table[b][square] = -bishop_st[square ^ 56];

        piece_square_table[R][square] = rook_st[square];
        piece_square_table[r][square] = -rook_st[square ^ 56];

        piece_square_table[Q][square] = queen_st[square];
        piece_square_table[q][square] = -queen_st[square ^ 56];

        piece_square_table[K][square] = king_st[square];
        piece_square_table[k][square] = -king_st[square ^ 56];
    }
}