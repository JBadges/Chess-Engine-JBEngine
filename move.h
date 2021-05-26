#pragma once

#include "types.h"
#include "bitboard.h"

namespace JACEA
{
    enum class MoveType
    {
        ALL,
        CAPTURES
    };

    constexpr int flag_capture = (1ULL << 16);
    constexpr int flag_enpassant = (1ULL << 17);
    constexpr int flag_double_pawn_push = (1ULL << 18);
    constexpr int flag_castle = (1ULL << 19);

    /**
     * Bits 0-5:    From square
     * Bits 6-11:   To square
     * Bits 12-15:  Promoted Piece
     * Bits 16+:    Flags
     */
    Move inline create_move(Square from, Square to, Piece promotedPiece, int flags)
    {
        return from | (to << 6) | (promotedPiece << 12) | flags;
    }

    inline Square get_from_square(const Move move)
    {
        return move & 0b111111;
    }

    inline Square get_to_square(const Move move)
    {
        return (move & 0b111111000000) >> 6;
    }

    inline Piece get_promoted_piece(const Move move)
    {
        return (move & 0b1111000000000000) >> 12;
    }

    inline int is_capture(const Move move)
    {
        return move & flag_capture;
    }

    inline int is_enpassant(const Move move)
    {
        return move & flag_enpassant;
    }

    inline int is_doublepawnpush(const Move move)
    {
        return move & flag_double_pawn_push;
    }

    inline int is_castle(const Move move)
    {
        return move & flag_castle;
    }

}