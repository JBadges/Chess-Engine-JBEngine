#include <assert.h>
#include "position.h"
#include "random.h"
#include <iostream>

using namespace JACEA;

u64 JACEA::piece_position_key[13][64];
u64 JACEA::side_key;
u64 JACEA::castle_perm_key[16];

void JACEA::init_zobrist_keys()
{
    for (Piece p = 0; p < 13; p++)
    {
        for (Square square = 0; square < 64; square++)
        {
            piece_position_key[p][square] = random_u64();
        }
    }
    for (int i = 0; i < 16; i++)
    {
        castle_perm_key[i] = random_u64();
    }
    side_key = random_u64();
}

void JACEA::Position::check() const
{
    // This function should only be run if in debug mode
#ifndef NDEBUG
    int white_val = 0;
    int black_val = 0;

    for (Square sq = 0; sq < 64; sq++)
    {
        if (mailbox[sq] == None)
            continue;
        assert(get_bit(piece_boards[mailbox[sq]], sq));
        if (color_from_piece[mailbox[sq]] == WHITE)
            white_val += piece_to_value[mailbox[sq]];
        else
            black_val += piece_to_value[mailbox[sq]];
    }

    assert(white_val == white_material);
    assert(black_val == black_material);

    Bitboard whitebb = 0ull;
    Bitboard blackbb = 0ull;
    Bitboard bothbb = 0ull;
    for (int piece = P; piece <= K; piece++)
    {
        whitebb |= piece_boards[piece];
    }

    for (int piece = p; piece <= k; piece++)
    {
        blackbb |= piece_boards[piece];
    }

    bothbb = whitebb | blackbb;

    assert(pop_count(piece_boards[p]) <= 8);
    assert(pop_count(piece_boards[P]) <= 8);
    assert(whitebb == occupancy[WHITE]);
    assert(blackbb == occupancy[BLACK]);
    assert(bothbb == occupancy[BOTH]);
    assert(zobrist_key == generate_zobrist_key());
#endif
}

u64 JACEA::Position::generate_zobrist_key() const
{
    u64 key = 0ULL;

    for (Square square = 0; square < 64; square++)
    {
        if (mailbox[square] == None)
            continue;
        key ^= piece_position_key[mailbox[square]][square];
    }

    if (side == WHITE)
        key ^= side_key;

    if (en_passant != no_sq)
        key ^= piece_position_key[None][en_passant];

    key ^= castle_perm_key[castling];

    return key;
}

JACEA::Position::Position()
{
    reset();
}

void JACEA::Position::reset()
{
    zobrist_key = 0ULL;
    for (int i = 0; i < 12; i++)
        piece_boards[i] = 0ULL;
    for (int i = 0; i < 3; i++)
        occupancy[i] = 0ULL;
    for (int i = 0; i < 64; i++)
        mailbox[i] = None;

    side = WHITE;
    en_passant = no_sq;
    castling = wk | wq | bk | bq;
    ply = 0;
    rule50 = 0;

    white_material = 0;
    black_material = 0;
    zobrist_key = generate_zobrist_key();
}

void JACEA::Position::init_from_fen(std::string fen)
{
    // Step 1: Clear position to ensure we are starting from the same object each time
    reset();

    auto c = fen.c_str();

    // Step 2: init piece bitboards from fen
    for (auto square = 0; square < 64; c++)
    {
        // If the current char is a number
        if ('0' <= *c && *c <= '9')
        {
            // Assuming the fen string is formatted properly, the file will never be overrun
            square += *c - '0';
        }
        // if A-z we need to add a piece to a specific bitboard
        else if (('A' <= *c && *c <= 'Z') || ('a' <= *c && *c <= 'z'))
        {
            assert(*c == 'P' || *c == 'N' || *c == 'B' || *c == 'R' || *c == 'Q' || *c == 'K' ||
                   *c == 'p' || *c == 'n' || *c == 'b' || *c == 'r' || *c == 'q' || *c == 'k');

            const int piece = char_to_piece(*c);
            add_piece(square, piece);
            square++;
        }
    }

    // Step 3: init side, castling, enPassant
    while (*c == ' ')
        c++;
    side = (*c++ == 'w') ? WHITE : BLACK;

    while (*++c != ' ')
    {
        switch (*c)
        {
        case 'K':
            castling |= wk;
            break;
        case 'Q':
            castling |= wq;
            break;
        case 'k':
            castling |= bk;
            break;
        case 'q':
            castling |= bq;
            break;
        case '-':
            castling = 0;
            break;
        default:
            assert(false);
        }
    }

    en_passant = no_sq;
    if (*++c != '-')
    {
        const int file = *c++ - 'a';
        const int rank = 8 - (*c++ - '0');
        const int square = rank * 8 + file;
        en_passant = square;
    }

    zobrist_key = generate_zobrist_key();
    check();
}

bool JACEA::Position::make_move(Move move) { return false; }

void JACEA::Position::print() const
{
    std::cout << std::endl;
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            const int square = rank * 8 + file;

            if (!file)
                printf(" %d ", 8 - rank);

            std::cout << " " << piece_to_string[mailbox[square]] << " ";
        }
        printf("\n");
    }
    printf("    a  b  c  d  e  f  g  h\n\n");
}