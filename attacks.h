#pragma once
#include "bitboard.h"

// [side][square]
extern Bitboard pawn_attacks[2][64];
extern Bitboard knight_attacks[64];
extern Bitboard king_attacks[64];

// Used for magic bitboard shift
extern int bishop_relevant_bits[64];
extern int rook_relevant_bits[64];

extern Bitboard bishop_mask[64];
extern Bitboard bishop_attacks[64][512];
extern Bitboard rook_mask[64];
extern Bitboard rook_attacks[64][4096];

// Magic numbers
extern u64 bishop_magics[64];
extern u64 rook_magics[64];

// Generate the static attack per square
void init_pawn_attacks();
void init_knight_attacks();
void init_king_attacks();

// Generate the static attack masks per square
void init_bishop_mask();
void init_rook_mask();

// Init relevancy masks
void init_relevancy_bishop();
void init_relevancy_rook();

// Init magic numbers
void init_magic_numbers();

// Init magic attacks
void init_bishop_magic_attack();
void init_rook_magic_attack();

u64 find_magic_number_bishop(const int square, const int relevant_bits);
u64 find_magic_number_rook(const int square, const int relevant_bits);

// Generate attack rays given a blocker
Bitboard generate_bishop_attacks(const int square, const Bitboard blocker);
Bitboard generate_rook_attacks(const int square, const Bitboard blocker);

Bitboard set_occupancy(const int index, const int bits_in_mask, Bitboard attack_mask);

static inline Bitboard get_bishop_attacks(const int square, u64 occupancy)
{
    assert(0 <= square && square < 64);
    occupancy &= bishop_mask[square];
    occupancy *= bishop_magics[square];
    occupancy >>= 64ull - bishop_relevant_bits[square];
    return bishop_attacks[square][occupancy];
}

static inline Bitboard get_rook_attacks(const int square, u64 occupancy)
{
    assert(0 <= square && square < 64);
    occupancy &= rook_mask[square];
    occupancy *= rook_magics[square];
    occupancy >>= 64ull - rook_relevant_bits[square];
    return rook_attacks[square][occupancy];
}

static inline Bitboard get_queen_attacks(const int square, u64 occupancy)
{
    assert(0 <= square && square < 64);
    return get_bishop_attacks(square, occupancy) | get_rook_attacks(square, occupancy);
}