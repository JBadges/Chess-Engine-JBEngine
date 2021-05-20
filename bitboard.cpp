#include "bitboard.h"
#include <iostream>

void JACEA::print_bitboard(const Bitboard bb)
{
    printf("\n");
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            const int square = rank * 8 + file;

            if (!file)
                printf(" %d ", 8 - rank);

            printf(" %d ", get_bit(bb, square) ? 1 : 0);
        }
        printf("\n");
    }
    printf("    a  b  c  d  e  f  g  h\n\n");
    printf("Bitboard: %llu \n", bb);
}