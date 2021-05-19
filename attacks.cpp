#include "attacks.h"
#include "types.h"
#include "random.h"
#include <cstring>
#include <iostream>

Bitboard pawn_attacks[2][64];
Bitboard knight_attacks[64];
Bitboard king_attacks[64];

int bishop_relevant_bits[64];
int rook_relevant_bits[64];

Bitboard bishop_mask[64];
Bitboard bishop_attacks[64][512];
Bitboard rook_mask[64];
Bitboard rook_attacks[64][4096];

u64 bishop_magics[64];
u64 rook_magics[64];

void init_pawn_attacks()
{
    for (int square = 0; square < 64; square++)
    {
        Bitboard b = 0ULL;
        set_bit(b, square);
        b = shift<UP_LEFT>(b) | shift<UP_RIGHT>(b);
        pawn_attacks[WHITE][square] = b;
        b = 0ULL;
        set_bit(b, square);
        b = shift<DOWN_LEFT>(b) | shift<DOWN_RIGHT>(b);
        pawn_attacks[BLACK][square] = b;
    }
}

void init_knight_attacks()
{
    for (int square = 0; square < 64; square++)
    {
        Bitboard b = 0ULL;
        set_bit(b, square);
        knight_attacks[square] = 0ULL;
        knight_attacks[square] |= shift<RIGHT>(shift<RIGHT>(shift<UP>(b)));
        knight_attacks[square] |= shift<RIGHT>(shift<RIGHT>(shift<DOWN>(b)));
        knight_attacks[square] |= shift<UP>(shift<UP>(shift<LEFT>(b)));
        knight_attacks[square] |= shift<UP>(shift<UP>(shift<RIGHT>(b)));
        knight_attacks[square] |= shift<LEFT>(shift<LEFT>(shift<UP>(b)));
        knight_attacks[square] |= shift<LEFT>(shift<LEFT>(shift<DOWN>(b)));
        knight_attacks[square] |= shift<DOWN>(shift<DOWN>(shift<LEFT>(b)));
        knight_attacks[square] |= shift<DOWN>(shift<DOWN>(shift<RIGHT>(b)));
    }
}

void init_king_attacks()
{
    for (int square = 0; square < 64; square++)
    {
        Bitboard b = 0ULL;
        set_bit(b, square);
        king_attacks[square] = 0ULL;
        king_attacks[square] |= shift<RIGHT>(b);
        king_attacks[square] |= shift<LEFT>(b);
        king_attacks[square] |= shift<UP>(b);
        king_attacks[square] |= shift<DOWN>(b);
        king_attacks[square] |= shift<UP_RIGHT>(b);
        king_attacks[square] |= shift<UP_LEFT>(b);
        king_attacks[square] |= shift<DOWN_RIGHT>(b);
        king_attacks[square] |= shift<DOWN_LEFT>(b);
    }
}

void init_bishop_mask()
{
    for (int square = 0; square < 64; square++)
    {
        Bitboard attack_mask = 0ULL;

        int rank, file;

        int to_rank = square / 8;
        int to_file = square % 8;

        for (rank = to_rank + 1, file = to_file + 1; rank < 7 && file < 7; rank++, file++)
            set_bit(attack_mask, rank * 8 + file);
        for (rank = to_rank + 1, file = to_file - 1; rank < 7 && file > 0; rank++, file--)
            set_bit(attack_mask, rank * 8 + file);
        for (rank = to_rank - 1, file = to_file + 1; rank > 0 && file < 7; rank--, file++)
            set_bit(attack_mask, rank * 8 + file);
        for (rank = to_rank - 1, file = to_file - 1; rank > 0 && file > 0; rank--, file--)
            set_bit(attack_mask, rank * 8 + file);

        bishop_mask[square] = attack_mask;
    }
}

void init_rook_mask()
{
    for (int square = 0; square < 64; square++)
    {
        Bitboard attack_mask = 0ULL;

        int rank, file;

        int to_rank = square / 8;
        int to_file = square % 8;

        for (rank = to_rank + 1; rank < 7; rank++)
            set_bit(attack_mask, rank * 8 + to_file);
        for (rank = to_rank - 1; rank > 0; rank--)
            set_bit(attack_mask, rank * 8 + to_file);
        for (file = to_file + 1; file < 7; file++)
            set_bit(attack_mask, to_rank * 8 + file);
        for (file = to_file - 1; file > 0; file--)
            set_bit(attack_mask, to_rank * 8 + file);

        rook_mask[square] = attack_mask;
    }
}

void init_relevancy_bishop()
{
    for (int i = 0; i < 64; i++)
    {
        bishop_relevant_bits[i] = pop_count(bishop_mask[i]);
    }
}

void init_relevancy_rook()
{
    for (int i = 0; i < 64; i++)
    {
        rook_relevant_bits[i] = pop_count(rook_mask[i]);
    }
}

void init_bishop_magic_attack()
{
    for (int square = 0; square < 64; square++)
    {
        Bitboard attack_mask = bishop_mask[square];

        int bits_count = pop_count(attack_mask);

        int occ_indicies = (1 << bits_count);

        for (int i = 0; i < occ_indicies; i++)
        {
            Bitboard occ = set_occupancy(i, bits_count, attack_mask);
            int magic_index = (occ * bishop_magics[square]) >> (64 - bishop_relevant_bits[square]);

            bishop_attacks[square][magic_index] = generate_bishop_attacks(square, occ);
        }
    }
}

void init_rook_magic_attack()
{
    for (int square = 0; square < 64; square++)
    {
        Bitboard attack_mask = rook_mask[square];

        int bits_count = pop_count(attack_mask);

        int occ_indicies = (1 << bits_count);

        for (int i = 0; i < occ_indicies; i++)
        {
            Bitboard occ = set_occupancy(i, bits_count, attack_mask);
            int magic_index = (occ * rook_magics[square]) >> (64 - rook_relevant_bits[square]);

            rook_attacks[square][magic_index] = generate_rook_attacks(square, occ);
        }
    }
}

void init_magic_numbers()
{
    for (int i = 0; i < 64; i++)
    {
        bishop_magics[i] = find_magic_number_bishop(i, bishop_relevant_bits[i]);
        rook_magics[i] = find_magic_number_rook(i, rook_relevant_bits[i]);
    }
}

u64 find_magic_number_bishop(const int square, const int relevant_bits)
{
    Bitboard occs[4096];
    Bitboard attacks[4096];
    Bitboard used_attacks[4096];

    Bitboard attack_mask = bishop_mask[square];

    int occ_indicies = (1 << relevant_bits);
    for (int i = 0; i < occ_indicies; i++)
    {
        occs[i] = set_occupancy(i, relevant_bits, attack_mask);

        attacks[i] = generate_bishop_attacks(square, occs[i]);
    }
    while (true)
    {
        u64 magic = JACEA::generate_magic_number();

        if (pop_count((attack_mask * magic) & 0xFF00000000000000) < 6)
            continue;

        memset(used_attacks, 0ULL, sizeof(used_attacks));

        bool fail = false;
        for (int i = 0; !fail && i < occ_indicies; i++)
        {
            int magic_index = (occs[i] * magic) >> (64 - relevant_bits);

            if (used_attacks[magic_index] == 0ULL)
            {
                used_attacks[magic_index] = attacks[i];
            }
            else if (used_attacks[magic_index] != attacks[i])
            {
                fail = true;
            }
        }
        if (!fail)
            return magic;
    }

    std::cout << "Error: Magic could not be found" << std::endl;
    return 0ULL;
}

u64 find_magic_number_rook(const int square, const int relevant_bits)
{
    Bitboard occs[4096];
    Bitboard attacks[4096];
    Bitboard used_attacks[4096];

    Bitboard attack_mask = rook_mask[square];

    int occ_indicies = (1 << relevant_bits);
    for (int i = 0; i < occ_indicies; i++)
    {
        occs[i] = set_occupancy(i, relevant_bits, attack_mask);

        attacks[i] = generate_rook_attacks(square, occs[i]);
    }
    while (true)
    {
        u64 magic = JACEA::generate_magic_number();

        if (pop_count((attack_mask * magic) & 0xFF00000000000000) < 6 || !magic)
            continue;

        memset(used_attacks, 0ULL, sizeof(used_attacks));

        bool fail = false;
        for (int i = 0; !fail && i < occ_indicies; i++)
        {
            int magic_index = (occs[i] * magic) >> (64 - relevant_bits);

            if (used_attacks[magic_index] == 0ULL)
            {
                used_attacks[magic_index] = attacks[i];
            }
            else if (used_attacks[magic_index] != attacks[i])
            {
                fail = true;
            }
        }
        if (!fail)
            return magic;
    }

    std::cout << "Error: Magic could not be found" << std::endl;
    return 0ULL;
}

Bitboard generate_bishop_attacks(const int square, const Bitboard blocker)
{
    Bitboard attack_mask = 0ULL;

    int rank, file;

    int to_rank = square / 8;
    int to_file = square % 8;

    for (rank = to_rank + 1, file = to_file + 1; rank <= 7 && file <= 7; rank++, file++)
    {
        set_bit(attack_mask, rank * 8 + file);
        if (get_bit(blocker, rank * 8 + file))
            break;
    }
    for (rank = to_rank + 1, file = to_file - 1; rank <= 7 && file >= 0; rank++, file--)
    {
        set_bit(attack_mask, rank * 8 + file);
        if (get_bit(blocker, rank * 8 + file))
            break;
    }
    for (rank = to_rank - 1, file = to_file + 1; rank >= 0 && file <= 7; rank--, file++)
    {
        set_bit(attack_mask, rank * 8 + file);
        if (get_bit(blocker, rank * 8 + file))
            break;
    }
    for (rank = to_rank - 1, file = to_file - 1; rank >= 0 && file >= 0; rank--, file--)
    {
        set_bit(attack_mask, rank * 8 + file);
        if (get_bit(blocker, rank * 8 + file))
            break;
    }

    return attack_mask;
}

Bitboard generate_rook_attacks(const int square, const Bitboard blocker)
{
    Bitboard attack_mask = 0ULL;

    int rank, file;

    int to_rank = square / 8;
    int to_file = square % 8;

    for (rank = to_rank + 1; rank <= 7; rank++)
    {
        set_bit(attack_mask, rank * 8 + to_file);
        if (get_bit(blocker, rank * 8 + to_file))
            break;
    }
    for (rank = to_rank - 1; rank >= 0; rank--)
    {
        set_bit(attack_mask, rank * 8 + to_file);
        if (get_bit(blocker, rank * 8 + to_file))
            break;
    }
    for (file = to_file + 1; file <= 7; file++)
    {
        set_bit(attack_mask, to_rank * 8 + file);
        if (get_bit(blocker, to_rank * 8 + file))
            break;
    }
    for (file = to_file - 1; file >= 0; file--)
    {
        set_bit(attack_mask, to_rank * 8 + file);
        if (get_bit(blocker, to_rank * 8 + file))
            break;
    }

    return attack_mask;
}

Bitboard set_occupancy(const int index, const int bits_in_mask, Bitboard attack_mask)
{
    Bitboard occupancy = 0ULL;

    for (int count = 0; count < bits_in_mask; count++)
    {
        const int square = get_firstlsb_index(attack_mask);

        if (get_bit(index, count))
            set_bit(occupancy, square);

        pop_bit(attack_mask, square);
    }

    return occupancy;
}