#include "attacks.h"
#include "bitboard.h"
#include "types.h"
#include "move.h"
#include <string>

namespace JACEA
{
    // Zobrist key random values
    extern u64 piece_position_key[13][64];
    extern u64 side_key;
    extern u64 castle_perm_key[16];
    extern int square_to_castle_perm[64];

    void init_zobrist_keys();

    constexpr int max_game_ply = 2 << 8;

    struct PositionHistory
    {
        u64 key;
        int castling;
        int rule50;
        int plies;
        Square en_passant;
        Piece captured_piece;
        Move move;
    };

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

        PositionHistory history[max_game_ply]; // Stores the previous move history to undo
        int history_size = 0;

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
        }

        inline bool is_square_attacked(const Color attacker, const Square square)
        {
            assert(attacker == WHITE || attacker == BLACK);
            assert(0 <= square && square < 64);

            // Pawns
            if (attacker == WHITE && pawn_attacks[BLACK][square] & piece_boards[P])
                return true;
            if (attacker == BLACK && pawn_attacks[WHITE][square] & piece_boards[p])
                return true;

            // Knights
            if (attacker == WHITE && knight_attacks[square] & piece_boards[N])
                return true;
            if (attacker == BLACK && knight_attacks[square] & piece_boards[n])
                return true;

            // King
            if (attacker == WHITE && king_attacks[square] & piece_boards[K])
                return true;
            if (attacker == BLACK && king_attacks[square] & piece_boards[k])
                return true;

            // Bishops
            if (attacker == WHITE && get_bishop_attacks(square, occupancy[BOTH]) & piece_boards[B])
                return true;
            if (attacker == BLACK && get_bishop_attacks(square, occupancy[BOTH]) & piece_boards[b])
                return true;

            // Rooks
            if (attacker == WHITE && get_rook_attacks(square, occupancy[BOTH]) & piece_boards[R])
                return true;
            if (attacker == BLACK && get_rook_attacks(square, occupancy[BOTH]) & piece_boards[r])
                return true;

            // Queens
            if (attacker == WHITE && get_queen_attacks(square, occupancy[BOTH]) & piece_boards[Q])
                return true;
            if (attacker == BLACK && get_queen_attacks(square, occupancy[BOTH]) & piece_boards[q])
                return true;

            return false;
        }

        u64 generate_zobrist_key() const;
        void check() const;

    public:
        Position();
        void reset();
        void init_from_fen(std::string fen);

        bool make_move(Move move, const MoveType mt);
        void take_move();

        void print() const;
    };
}