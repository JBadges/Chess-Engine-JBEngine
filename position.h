#pragma once

#include "attacks.h"
#include "bitboard.h"
#include "types.h"
#include "move.h"
#include <string>
#include <cstring>
#include <iostream>

namespace JACEA
{

    // Zobrist key random values
    extern u64 piece_position_key[13][64];
    extern u64 side_key;
    extern u64 castle_perm_key[16];
    extern int square_to_castle_perm[64];

    void init_zobrist_keys();

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

        int killer_moves[2][max_game_ply];
        //[piece][square] score of move
        int history_moves[12][64];

        int pv_length[max_game_ply];
        Move pv_table[max_game_ply][max_game_ply];

        bool follow_pv;
        bool score_pv;

        inline void add_piece(const Piece piece, const Square square)
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
            take_piece(from_square);
            add_piece(piece, to_square);
        }

        u64 generate_zobrist_key() const;
        void check() const;

    public:
        Position();
        Position &operator=(const Position &rhs);
        void reset();
        void init_from_fen(std::string fen);

        bool make_move(Move move, const MoveType mt);
        void take_move();
        void make_null_move();
        void take_null_move();

        void print() const;

        inline void reset_ply() { ply = 0; }

        inline Color get_side() const { return side; }
        inline Square get_enpassant_square() const { return en_passant; }
        inline int get_castling_perms() const { return castling; }
        inline int get_material_white() const { return white_material; }
        inline int get_material_black() const { return black_material; }
        inline int get_ply() const { return ply; }
        inline int get_fifty() const { return rule50; }
        inline u64 get_key() const { return zobrist_key; }
        inline int get_total_moves() const { return history_size; }
        inline Bitboard get_piece_on_square(const Square square) const { return mailbox[square]; }
        inline Bitboard get_piece_board(const Piece piece) const { return piece_boards[piece]; }
        inline Bitboard get_occupancy_board(const Color color) const { return occupancy[color]; }
        inline Move get_first_killer_move() const { return killer_moves[0][ply]; }
        inline Move get_second_killer_move() const { return killer_moves[1][ply]; }
        inline Move get_history_move(const Piece piece, const Square square) const { return history_moves[piece][square]; }
        inline bool is_square_attacked(const Color attacker, const Square square) const
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
        inline bool three_fold_repetition()
        {
            int r = 0;
            for (int i = 0; i < history_size; i++)
            {
                if (zobrist_key == history[i].key)
                    r++;
                if (r >= 2)
                    return true;
            }
            return false;
        }
        inline bool draw()
        {
            return three_fold_repetition() || rule50 >= 100;
        }
        inline void update_killer(const Move move)
        {
            killer_moves[1][ply] = killer_moves[0][ply];
            killer_moves[0][ply] = move;
        }
        inline void init_search()
        {
            follow_pv = false;
            score_pv = false;

            std::memset(killer_moves, 0, sizeof(killer_moves));
            std::memset(history_moves, 0, sizeof(history_moves));
            std::memset(pv_table, 0, sizeof(pv_table));
            std::memset(pv_length, 0, sizeof(pv_length));
        }
        inline void update_history(const Move move, const int depth)
        {
            history_moves[get_piece_on_square(get_from_square(move))][get_to_square(move)] += depth;
        }
        inline void update_current_pv_length()
        {
            pv_length[ply] = ply;
        }
        inline void update_pv(const Move move)
        {
            pv_table[ply][ply] = move;

            for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
            {
                pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
            }

            pv_length[ply] = pv_length[ply + 1];
        }
        inline void update_follow_pv(const MoveList &ml)
        {
            if (follow_pv)
            {
                follow_pv = false;
                for (int i = 0; i < ml.size; i++)
                {
                    if (pv_table[0][ply] == ml.moves[i].move)
                    {
                        score_pv = true;
                        follow_pv = true;
                    }
                }
            }
        }
        inline void follow_pv_true() { follow_pv = true; }
        inline void set_score_pv(const bool b) { score_pv = b; }
        inline Move get_pv_best() { return pv_table[0][0]; }
        inline Move get_pv_ply() { return pv_table[0][ply]; }
        inline bool get_should_score() { return score_pv; }
        inline void print_pv_line()
        {
            for (int i = 0; i < pv_length[0]; i++)
            {
                Move move = pv_table[0][i];
                std::cout << square_to_coordinate[get_from_square(move)] << square_to_coordinate[get_to_square(move)];
                if (get_promoted_piece(move) != 0)
                    std::cout << piece_to_string[get_promoted_piece(move)];
                std::cout << " ";
            }
        }
    };
}