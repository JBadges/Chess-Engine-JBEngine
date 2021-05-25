#include "bitboard.h"
#include "types.h"
#include <string>

namespace JACEA
{
    // Zobrist key random values
    extern u64 piece_position_key[13][64];
    extern u64 side_key;
    extern u64 castle_perm_key[16];

    void init_zobrist_keys();

    class Position
    {
    private:
        /**
         *  Position information
         */
        u64 zobrist_key;           // Mostly unique position identifier
        Bitboard piece_boards[12]; // Indexed by ePiece enum
        Bitboard occupancy[3];     // Index by eColor
        Piece mailbox[64];         // Each index represents the tile of enum eSquare
        Color side;                // Whos side is it to turn
        Square en_passant;         // The current en_passant square, set to no_sq if not avaiable
        int castling;              // uses first 4 bits of data to hold castling perms of both sides
        int ply;                   // ply is increased after each move made
        int rule50;                // Used for calling draws after no pawn move or capture in 50 moves

        /**
         *  Eval
         */
        int white_material; // white material score
        int black_material; // black material score

        inline void add_piece(const Square square, const Piece piece)
        {
            assert(mailbox[square] == None);
            assert(piece != None);

            mailbox[square] = piece;
            set_bit(piece_boards[piece], square);
            set_bit(occupancy[color_from_piece[piece]], square);
            set_bit(occupancy[BOTH], square);

            if (color_from_piece[piece] == WHITE)
                white_material += piece_to_value[piece];
            else
                black_material += piece_to_value[piece];

            zobrist_key ^= piece_position_key[piece][square];

            check();
        }

        inline void take_piece(const Square square)
        {
            assert(mailbox[square] != None);
            const Piece piece = mailbox[square];

            mailbox[square] = None;
            pop_bit(piece_boards[piece], square);
            pop_bit(occupancy[color_from_piece[piece]], square);
            pop_bit(occupancy[BOTH], square);

            if (color_from_piece[piece] == WHITE)
                white_material -= piece_to_value[piece];
            else
                black_material -= piece_to_value[piece];

            zobrist_key ^= piece_position_key[piece][square];

            check();
        }

        // To square must be empty. (Does not handle captures)
        inline void move_piece(const Square from_square, const Square to_square)
        {
            assert(mailbox[from_square] != None);
            assert(mailbox[to_square] == None);
            const Piece piece = mailbox[from_square];

            mailbox[to_square] = piece;
            mailbox[from_square] = None;

            Bitboard from_bb = 1ULL << from_square;
            Bitboard to_bb = 1ULL << to_square;
            Bitboard from_to_bb = from_bb | to_bb;

            piece_boards[piece] ^= from_to_bb;
            occupancy[color_from_piece[piece]] ^= from_to_bb;
            occupancy[BOTH] ^= from_to_bb;

            zobrist_key ^= piece_position_key[piece][from_square];
            zobrist_key ^= piece_position_key[piece][to_square];

            check();
        }

        u64 generate_zobrist_key() const;
        void check() const;

    public:
        Position();
        void reset();
        void init_from_fen(std::string fen);

        bool make_move(Move move);

        void print() const;
    };
}