#pragma once

#include <assert.h>

namespace JACEA
{
    typedef unsigned long long u64;
    // All Bitboards are of type unsigned long long. For readability, having type bitboard allows for better understanding
    typedef u64 Bitboard;

    enum Direction
    {
        UP,
        DOWN,
        RIGHT,
        LEFT,
        UP_RIGHT,
        UP_LEFT,
        DOWN_RIGHT,
        DOWN_LEFT
    };

    typedef int Square;

    enum eSquare : int
    {
        no_sq = -1,
        a8 = 0,
        b8,
        c8,
        d8,
        e8,
        f8,
        g8,
        h8,
        a7,
        b7,
        c7,
        d7,
        e7,
        f7,
        g7,
        h7,
        a6,
        b6,
        c6,
        d6,
        e6,
        f6,
        g6,
        h6,
        a5,
        b5,
        c5,
        d5,
        e5,
        f5,
        g5,
        h5,
        a4,
        b4,
        c4,
        d4,
        e4,
        f4,
        g4,
        h4,
        a3,
        b3,
        c3,
        d3,
        e3,
        f3,
        g3,
        h3,
        a2,
        b2,
        c2,
        d2,
        e2,
        f2,
        g2,
        h2,
        a1,
        b1,
        c1,
        d1,
        e1,
        f1,
        g1,
        h1
    };

    constexpr Bitboard NOT_A_FILE = 18374403900871474942ULL;
    constexpr Bitboard NOT_B_FILE = 18302063728033398269ULL;
    constexpr Bitboard NOT_AB_FILE = NOT_A_FILE & NOT_B_FILE;
    constexpr Bitboard NOT_G_FILE = 13816973012072644543ULL;
    constexpr Bitboard NOT_H_FILE = 9187201950435737471ULL;
    constexpr Bitboard NOT_GH_FILE = NOT_G_FILE & NOT_H_FILE;
    constexpr Bitboard SECOND_RANK = 71776119061217280ULL;
    constexpr Bitboard SEVENTH_RANK = 65280ULL;
    constexpr Bitboard WHITE_QUEEN_CASTLE = 1008806316530991104ULL;
    constexpr Bitboard WHITE_KING_CASTLE = 6917529027641081856ULL;
    constexpr Bitboard BLACK_QUEEN_CASTLE = 14ULL;
    constexpr Bitboard BLACK_KING_CASTLE = 96ULL;

    inline Bitboard get_bit(const Bitboard &bb, const Square square)
    {
        assert(0 <= square && square < 64);
        return bb & (1ULL << square);
    }

    inline Bitboard set_bit(Bitboard &bb, const Square square)
    {
        assert(0 <= square && square < 64);
        return bb |= (1ULL << square);
    }

    inline int pop_count(const Bitboard &bb)
    {
        return __builtin_popcountll(bb);
    }

    inline Bitboard pop_bit(Bitboard &bb, const Square square)
    {
        assert(0 <= square && square < 64);
        return bb &= ~(1ULL << square);
    }

    template <Direction dir>
    inline Bitboard shift(const Bitboard bb)
    {
        switch (dir)
        {
        case UP:
            return bb >> 8ULL;
        case DOWN:
            return bb << 8ULL;
        case RIGHT:
            return (bb & NOT_H_FILE) << 1ULL;
        case LEFT:
            return (bb & NOT_A_FILE) >> 1ULL;
        case UP_RIGHT:
            return (bb & NOT_H_FILE) >> 7ULL;
        case UP_LEFT:
            return (bb & NOT_A_FILE) >> 9ULL;
        case DOWN_RIGHT:
            return (bb & NOT_H_FILE) << 9ULL;
        case DOWN_LEFT:
            return (bb & NOT_A_FILE) << 7ULL;
        default:
            assert(false);
            break;
        }
    }

    inline int get_firstlsb_index(const Bitboard bb)
    {
        assert(bb);
        return pop_count((bb & -bb) - 1);
    }

    void print_bitboard(const Bitboard bb);
}