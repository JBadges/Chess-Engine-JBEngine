#pragma once
#include "position.h"
#include "types.h"

namespace JACEA
{

    extern int mvv_lva[12][12];

    void init_mvv_lva();

    static inline int score_move(Position &pos, const Move move, const Move best_move = 0)
    {
        if (move == best_move)
        {
            return 1000000;
        }
        if (pos.get_should_score())
        {
            if (pos.get_pv_ply() == move)
            {
                pos.set_score_pv(false);
                return 200000;
            }
        }
        if (is_capture(move))
        {
            int piece = pos.get_piece_on_square(get_from_square(move));
            int captured_piece = pos.get_piece_on_square(get_to_square(move));
            return mvv_lva[captured_piece][piece] + 10000;
        }
        else if (is_enpassant(move))
        {
            return mvv_lva[P][p] + 10000;
        }
        else
        {
            if (pos.get_first_killer_move() == move)
                return 9000;
            else if (pos.get_second_killer_move() == move)
                return 8000;
            else
                return pos.get_history_move(pos.get_piece_on_square(get_from_square(move)), get_to_square(move));
        }

        return 0;
    }

    static inline void add_move(Position &pos, MoveList &ml, const Move move, const Move best_move = 0)
    {
        ml.moves[ml.size++] = {move, score_move(pos, move, best_move)};
    }

    static inline void generate_moves(Position &pos, MoveList &ml, const Move best_move = 0)
    {
        if (pos.get_side() == WHITE)
        {
            for (int piece = P; piece <= K; piece++)
            {
                u64 bb = pos.get_piece_board(piece);
                switch (piece)
                {
                case P:
                    // Single pawn pushes
                    {
                        // White pawns getting shifted up one rank gives all quiet pawn pushes
                        u64 single_pawn_push_moves = shift<Direction::UP>(bb) & ~pos.get_occupancy_board(BOTH);
                        while (single_pawn_push_moves)
                        {
                            const int to_square = get_firstlsb_index(single_pawn_push_moves);

                            // Promotion
                            if (a8 <= to_square && to_square <= h8)
                            {
                                add_move(pos, ml, create_move(to_square + 8, to_square, Q, 0), best_move);
                                add_move(pos, ml, create_move(to_square + 8, to_square, N, 0), best_move);
                                add_move(pos, ml, create_move(to_square + 8, to_square, B, 0), best_move);
                                add_move(pos, ml, create_move(to_square + 8, to_square, R, 0), best_move);
                            }
                            else
                            {
                                add_move(pos, ml, create_move(to_square + 8, to_square, 0, 0), best_move);
                            }

                            pop_bit(single_pawn_push_moves, to_square);
                        }
                    }
                    // Double pawn push
                    {
                        // White pawns getting shifted up one rank gives all quiet pawn pushes
                        u64 double_pawn_push_moves = shift<Direction::UP>(shift<Direction::UP>(bb & SECOND_RANK) & ~pos.get_occupancy_board(BOTH)) & ~pos.get_occupancy_board(BOTH);
                        while (double_pawn_push_moves)
                        {
                            const int to_square = get_firstlsb_index(double_pawn_push_moves);

                            add_move(pos, ml, create_move(to_square + 16, to_square, 0, flag_double_pawn_push), best_move);

                            pop_bit(double_pawn_push_moves, to_square);
                        }
                    }
                    // Pawn attacks
                    {
                        u64 pawns = bb;
                        while (pawns)
                        {
                            const int from_square = get_firstlsb_index(pawns);
                            u64 attacks = pawn_attacks[WHITE][from_square] & pos.get_occupancy_board(BLACK);

                            while (attacks)
                            {
                                const int to_square = get_firstlsb_index(attacks);

                                // Promotion
                                if (a8 <= to_square && to_square <= h8)
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, Q, flag_capture), best_move);
                                    add_move(pos, ml, create_move(from_square, to_square, N, flag_capture), best_move);
                                    add_move(pos, ml, create_move(from_square, to_square, B, flag_capture), best_move);
                                    add_move(pos, ml, create_move(from_square, to_square, R, flag_capture), best_move);
                                }
                                else
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, flag_capture), best_move);
                                }

                                pop_bit(attacks, to_square);
                            }

                            if (pos.get_enpassant_square() != no_sq)
                            {
                                u64 enpassant_attack = pawn_attacks[WHITE][from_square] & (1ULL << pos.get_enpassant_square());
                                if (enpassant_attack)
                                {
                                    add_move(pos, ml, create_move(from_square, get_firstlsb_index(enpassant_attack), 0, flag_enpassant), best_move);
                                }
                            }

                            pop_bit(pawns, from_square);
                        }
                    }
                    break;
                case N:
                    // Knight
                    {
                        u64 knights = bb;
                        while (knights)
                        {
                            const int from_square = get_firstlsb_index(knights);
                            u64 attacks = knight_attacks[from_square];
                            attacks &= ~pos.get_occupancy_board(WHITE);
                            while (attacks)
                            {
                                const int to_square = get_firstlsb_index(attacks);

                                // Capture Move
                                if (get_bit(pos.get_occupancy_board(BLACK), to_square))
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, flag_capture), best_move);
                                }
                                // Quiet Move
                                else
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, 0), best_move);
                                }

                                pop_bit(attacks, to_square);
                            }

                            pop_bit(knights, from_square);
                        }
                    }
                    break;
                case B:
                    // Bishop
                    {
                        u64 bishop = bb;
                        while (bishop)
                        {
                            const int from_square = get_firstlsb_index(bishop);
                            u64 attacks = get_bishop_attacks(from_square, pos.get_occupancy_board(BOTH)) & ~pos.get_occupancy_board(WHITE);
                            while (attacks)
                            {
                                const int to_square = get_firstlsb_index(attacks);
                                // Capture Move
                                if (get_bit(pos.get_occupancy_board(BLACK), to_square))
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, flag_capture), best_move);
                                }
                                // Quiet Move
                                else
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, 0), best_move);
                                }

                                pop_bit(attacks, to_square);
                            }

                            pop_bit(bishop, from_square);
                        }
                    }
                    break;
                case R:
                    // Rook
                    {
                        u64 rooks = bb;
                        while (rooks)
                        {
                            const int from_square = get_firstlsb_index(rooks);
                            u64 attacks = get_rook_attacks(from_square, pos.get_occupancy_board(BOTH)) & ~pos.get_occupancy_board(WHITE);
                            while (attacks)
                            {
                                const int to_square = get_firstlsb_index(attacks);

                                // Capture Move
                                if (get_bit(pos.get_occupancy_board(BLACK), to_square))
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, flag_capture), best_move);
                                }
                                // Quiet Move
                                else
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, 0), best_move);
                                }

                                pop_bit(attacks, to_square);
                            }

                            pop_bit(rooks, from_square);
                        }
                    }
                    break;
                case Q:
                    // Queen
                    {
                        u64 queens = bb;
                        while (queens)
                        {
                            const int from_square = get_firstlsb_index(queens);
                            u64 attacks = get_queen_attacks(from_square, pos.get_occupancy_board(BOTH)) & ~pos.get_occupancy_board(WHITE);
                            while (attacks)
                            {
                                const int to_square = get_firstlsb_index(attacks);

                                // Capture Move
                                if (get_bit(pos.get_occupancy_board(BLACK), to_square))
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, flag_capture), best_move);
                                }
                                // Quiet Move
                                else
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, 0), best_move);
                                }

                                pop_bit(attacks, to_square);
                            }

                            pop_bit(queens, from_square);
                        }
                    }
                    break;
                case K:
                    // Castling
                    {
                        if ((pos.get_castling_perms() & wk) && !(pos.get_occupancy_board(BOTH) & WHITE_KING_CASTLE))
                        {
                            if (!pos.is_square_attacked(BLACK, e1) && !pos.is_square_attacked(BLACK, f1))
                            {
                                add_move(pos, ml, create_move(e1, g1, 0, flag_castle), best_move);
                            }
                        }
                        if ((pos.get_castling_perms() & wq) && !(pos.get_occupancy_board(BOTH) & WHITE_QUEEN_CASTLE))
                        {
                            if (!pos.is_square_attacked(BLACK, e1) && !pos.is_square_attacked(BLACK, d1))
                            {
                                add_move(pos, ml, create_move(e1, c1, 0, flag_castle), best_move);
                            }
                        }
                    }
                    // Moves
                    {
                        u64 king = bb;
                        const int from_square = get_firstlsb_index(king);
                        u64 attacks = king_attacks[from_square];
                        attacks &= ~pos.get_occupancy_board(WHITE);
                        while (attacks)
                        {
                            const int to_square = get_firstlsb_index(attacks);

                            // Capture Move
                            if (get_bit(pos.get_occupancy_board(BLACK), to_square))
                            {
                                add_move(pos, ml, create_move(from_square, to_square, 0, flag_capture), best_move);
                            }
                            // Quiet Move
                            else
                            {
                                add_move(pos, ml, create_move(from_square, to_square, 0, 0), best_move);
                            }

                            pop_bit(attacks, to_square);
                        }
                    }
                    break;
                default:
                    assert(false);
                }
            }
        }
        else
        {
            for (int piece = p; piece <= k; piece++)
            {
                u64 bb = pos.get_piece_board(piece);
                switch (piece)
                {
                case p:
                    // Single pawn pushes
                    {
                        // White pawns getting shifted up one rank gives all quiet pawn pushes
                        u64 single_pawn_push_moves = shift<Direction::DOWN>(bb) & ~pos.get_occupancy_board(BOTH);
                        while (single_pawn_push_moves)
                        {
                            const int to_square = get_firstlsb_index(single_pawn_push_moves);

                            // Promotion
                            if (a1 <= to_square && to_square <= h1)
                            {
                                add_move(pos, ml, create_move(to_square - 8, to_square, q, 0), best_move);
                                add_move(pos, ml, create_move(to_square - 8, to_square, n, 0), best_move);
                                add_move(pos, ml, create_move(to_square - 8, to_square, b, 0), best_move);
                                add_move(pos, ml, create_move(to_square - 8, to_square, r, 0), best_move);
                            }
                            else
                            {
                                add_move(pos, ml, create_move(to_square - 8, to_square, 0, 0), best_move);
                            }

                            pop_bit(single_pawn_push_moves, to_square);
                        }
                    }
                    // Double pawn push
                    {
                        // White pawns getting shifted up one rank gives all quiet pawn pushes
                        u64 double_pawn_push_moves = shift<Direction::DOWN>(shift<Direction::DOWN>(bb & SEVENTH_RANK) & ~pos.get_occupancy_board(BOTH)) & ~pos.get_occupancy_board(BOTH);
                        while (double_pawn_push_moves)
                        {
                            const int to_square = get_firstlsb_index(double_pawn_push_moves);

                            add_move(pos, ml, create_move(to_square - 16, to_square, 0, flag_double_pawn_push), best_move);

                            pop_bit(double_pawn_push_moves, to_square);
                        }
                    }
                    // Pawn attacks
                    {
                        u64 pawns = bb;
                        while (pawns)
                        {
                            const int from_square = get_firstlsb_index(pawns);
                            u64 attacks = pawn_attacks[BLACK][from_square] & pos.get_occupancy_board(WHITE);

                            while (attacks)
                            {
                                const int to_square = get_firstlsb_index(attacks);

                                // Promotion
                                if (a1 <= to_square && to_square <= h1)
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, q, flag_capture), best_move);
                                    add_move(pos, ml, create_move(from_square, to_square, n, flag_capture), best_move);
                                    add_move(pos, ml, create_move(from_square, to_square, b, flag_capture), best_move);
                                    add_move(pos, ml, create_move(from_square, to_square, r, flag_capture), best_move);
                                }
                                else
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, flag_capture), best_move);
                                }

                                pop_bit(attacks, to_square);
                            }

                            if (pos.get_enpassant_square() != no_sq)
                            {
                                u64 enpassant_attack = pawn_attacks[BLACK][from_square] & (1ULL << pos.get_enpassant_square());
                                if (enpassant_attack)
                                {
                                    add_move(pos, ml, create_move(from_square, get_firstlsb_index(enpassant_attack), 0, flag_enpassant), best_move);
                                }
                            }

                            pop_bit(pawns, from_square);
                        }
                    }
                    break;
                case n:
                    // Knight
                    {
                        u64 knights = bb;
                        while (knights)
                        {
                            const int from_square = get_firstlsb_index(knights);
                            u64 attacks = knight_attacks[from_square];
                            attacks &= ~pos.get_occupancy_board(BLACK);
                            while (attacks)
                            {
                                const int to_square = get_firstlsb_index(attacks);

                                // Capture Move
                                if (get_bit(pos.get_occupancy_board(WHITE), to_square))
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, flag_capture), best_move);
                                }
                                // Quiet Move
                                else
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, 0), best_move);
                                }

                                pop_bit(attacks, to_square);
                            }

                            pop_bit(knights, from_square);
                        }
                    }
                    break;
                case b:
                    // Bishop
                    {
                        u64 bishop = bb;
                        while (bishop)
                        {
                            const int from_square = get_firstlsb_index(bishop);
                            u64 attacks = get_bishop_attacks(from_square, pos.get_occupancy_board(BOTH)) & ~pos.get_occupancy_board(BLACK);
                            while (attacks)
                            {
                                const int to_square = get_firstlsb_index(attacks);

                                // Capture Move
                                if (get_bit(pos.get_occupancy_board(WHITE), to_square))
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, flag_capture), best_move);
                                }
                                // Quiet Move
                                else
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, 0), best_move);
                                }

                                pop_bit(attacks, to_square);
                            }

                            pop_bit(bishop, from_square);
                        }
                    }
                    break;
                case r:
                    // Rook
                    {
                        u64 rook = bb;
                        while (rook)
                        {
                            const int from_square = get_firstlsb_index(rook);
                            u64 attacks = get_rook_attacks(from_square, pos.get_occupancy_board(BOTH)) & ~pos.get_occupancy_board(BLACK);
                            while (attacks)
                            {
                                const int to_square = get_firstlsb_index(attacks);

                                // Capture Move
                                if (get_bit(pos.get_occupancy_board(WHITE), to_square))
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, flag_capture), best_move);
                                }
                                // Quiet Move
                                else
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, 0), best_move);
                                }

                                pop_bit(attacks, to_square);
                            }

                            pop_bit(rook, from_square);
                        }
                    }
                    break;
                case q:
                    // Queen
                    {
                        u64 queens = bb;
                        while (queens)
                        {
                            const int from_square = get_firstlsb_index(queens);
                            u64 attacks = get_queen_attacks(from_square, pos.get_occupancy_board(BOTH)) & ~pos.get_occupancy_board(BLACK);
                            while (attacks)
                            {
                                const int to_square = get_firstlsb_index(attacks);

                                // Capture Move
                                if (get_bit(pos.get_occupancy_board(WHITE), to_square))
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, flag_capture), best_move);
                                }
                                // Quiet Move
                                else
                                {
                                    add_move(pos, ml, create_move(from_square, to_square, 0, 0), best_move);
                                }

                                pop_bit(attacks, to_square);
                            }

                            pop_bit(queens, from_square);
                        }
                    }
                    break;
                case k:
                    // Castling
                    {
                        if ((pos.get_castling_perms() & bk) && !(pos.get_occupancy_board(BOTH) & BLACK_KING_CASTLE))
                        {
                            if (!pos.is_square_attacked(WHITE, e8) && !pos.is_square_attacked(WHITE, f8))
                            {
                                add_move(pos, ml, create_move(e8, g8, 0, flag_castle), best_move);
                            }
                        }
                        if ((pos.get_castling_perms() & bq) && !(pos.get_occupancy_board(BOTH) & BLACK_QUEEN_CASTLE))
                        {
                            if (!pos.is_square_attacked(WHITE, e8) && !pos.is_square_attacked(WHITE, d8))
                            {
                                add_move(pos, ml, create_move(e8, c8, 0, flag_castle), best_move);
                            }
                        }
                    }
                    // Moves
                    {
                        u64 king = bb;
                        const int from_square = get_firstlsb_index(king);
                        u64 attacks = king_attacks[from_square];
                        attacks &= ~pos.get_occupancy_board(BLACK);
                        while (attacks)
                        {
                            const int to_square = get_firstlsb_index(attacks);

                            // Capture Move
                            if (get_bit(pos.get_occupancy_board(WHITE), to_square))
                            {
                                add_move(pos, ml, create_move(from_square, to_square, 0, flag_capture), best_move);
                            }
                            // Quiet Move
                            else
                            {
                                add_move(pos, ml, create_move(from_square, to_square, 0, 0), best_move);
                            }

                            pop_bit(attacks, to_square);
                        }
                    }
                    break;
                default:
                    assert(false);
                }
            }
        }
    }
}