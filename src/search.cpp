#include "search.h"
#include "eval.h"
#include "movegenerator.h"
#include "utility.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include "random.h"
#include "tbprobe.h"
#include <future>

using namespace JACEA;

static inline bool compareDescendingMoves(const ScoredMove &a, const ScoredMove &b)
{
	return a.score > b.score;
}

static inline void update_stop(UCISettings &uci)
{
	uci.stop |= uci.completed_iteration && get_time_ms() > uci.time_to_stop;
}

static inline int quiesence(bool mainThread, JACEA::Position &pos, int alpha, int beta, UCISettings &uci)
{
	if (mainThread)
	{
		uci.nodes++;
		uci.largest_depth = std::max(uci.largest_depth, pos.get_ply());
	}

	if (uci.nodes & 15000) // ~ each ms
	{
		update_stop(uci);
	}
	if (uci.stop)
	{
		return 0;
	}
	if (pos.get_ply() >= max_game_ply)
		return evaluation(pos);

	int eval = evaluation(pos);

	if (eval >= beta)
		return beta;

	if (eval > alpha)
	{
		alpha = eval;
	}

	MoveList ml;
	generate_moves(pos, ml);
	std::sort(ml.moves, ml.moves + ml.size, compareDescendingMoves);
	for (int i = 0; i < ml.size; i++)
	{
		if (!pos.make_move(ml.moves[i].move, MoveType::CAPTURES))
			continue;

		const int score = -quiesence(mainThread, pos, -beta, -alpha, uci);

		pos.take_move();

		// PV move found
		if (score > alpha)
		{
			alpha = score;

			// Fail-hard (failed high)
			if (score >= beta)
				return beta;
		}
	}

	// failed low
	return alpha;
}

static inline int negamax(bool mainThread, JACEA::Position &pos, int alpha, int beta, int depth, TranspositionTable &tt, UCISettings &uci)
{
	bool pv_node = (beta - alpha) > 1;
	int eval = evaluation(pos);
	int score;
	int flag_hash = TranspositionTable::flag_hash_alpha;
	bool skip_quiet_moves = false;
	int reduction;

	if (uci.stop_threads)
		return 0;
	pos.update_current_pv_length();

	if (mainThread)
	{
		uci.nodes++;
		uci.largest_depth = std::max(uci.largest_depth, pos.get_ply());
	}

	if ((uci.nodes & 2048) == 0) // ~ each ms
	{
		update_stop(uci);
	}

	if (uci.stop)
	{
		return 0;
	}

	if (depth <= 0)
	{
		return quiesence(mainThread, pos, alpha, beta, uci);
	}

	if (pos.get_ply())
	{
		// Draw - return a non-zero value to help
		// detect drawing lines in a winning position
		if (pos.get_ply() != 0 && pos.draw())
		{
			return 1 - (uci.nodes & 2);
		}

		if (pos.get_ply() >= max_game_ply)
		{
			return evaluation(pos);
		}

		// Mate distance pruning
		alpha = std::max(mated_in(pos.get_ply()), alpha);
		beta = std::min(mate_in(pos.get_ply() + 1), beta);
		if (alpha >= beta)
			return alpha;

		// Transposition table lookup
		if (((score = tt.read_hash_entry(pos, alpha, beta, depth)) != TranspositionTable::no_hash) && !pv_node && pos.get_fifty() < 90)
		{
			return score;
		}

		// Tablebase lookup
		if (pop_count(pos.get_occupancy_board(BOTH)) <= 5)
		{
			if (mainThread)
				uci.table_base_hits++;

			unsigned res = tb_probe_wdl(bswap64(pos.get_occupancy_board(WHITE)),
										bswap64(pos.get_occupancy_board(BLACK)),
										bswap64(pos.get_piece_board(k) | pos.get_piece_board(K)),
										bswap64(pos.get_piece_board(q) | pos.get_piece_board(Q)),
										bswap64(pos.get_piece_board(r) | pos.get_piece_board(R)),
										bswap64(pos.get_piece_board(b) | pos.get_piece_board(B)),
										bswap64(pos.get_piece_board(n) | pos.get_piece_board(N)),
										bswap64(pos.get_piece_board(p) | pos.get_piece_board(P)),
										pos.get_fifty(),
										pos.get_castling_perms(),
										pos.get_enpassant_square() == no_sq ? 0 : pos.get_enpassant_square(),
										pos.get_side() ^ 1);

			if (res != TB_RESULT_FAILED)
			{
				unsigned wdl = TB_GET_WDL(res);
				switch (wdl)
				{
					// Lost
				case 0:
					return mated_in(pos.get_ply());
					break;
					// Draw
				case 1:
				case 2:
				case 3:
					return 0;
					break;
					// Win
				case 4:
					return mate_in(pos.get_ply());
					break;
				default:
					break;
				}
			}
		}
	}

	bool in_check = pos.is_square_attacked(pos.get_side() ^ 1, (pos.get_side() == WHITE) ? get_firstlsb_index(pos.get_piece_board(K)) : get_firstlsb_index(pos.get_piece_board(k)));

	if (in_check)
		depth++;

	// Razoring
	int razor_value = eval + 125;
	if (pos.get_ply() && razor_value < beta)
	{
		if (depth == 1)
		{
			int new_val = quiesence(mainThread, pos, alpha, beta, uci);
			return std::max(new_val, razor_value);
		}
		razor_value += 175;
		if (razor_value < beta && depth <= 3)
		{
			int new_val = quiesence(mainThread, pos, alpha, beta, uci);
			if (new_val < beta)
			{
				return std::max(new_val, razor_value);
			}
		}
	}

	// Beta Pruning - If the eval is well above beta we assume
	// that it will still hold at a higher depth
	// if (pos.get_ply() && !pv_node && !in_check && depth <= 8 && eval - 75 * depth > beta)
	// 	return eval;

	// Alpha Pruning - If the move is so bad now, it will probably be bad
	// even at high depths
	// if (pos.get_ply() && !pv_node && !in_check && depth <= 5 && eval + 3000 <= alpha)
	// 	return eval;

	// Futility pruning
	// if (pos.get_ply() && !pv_node && depth < 9 && eval - (200 * depth) >= beta && eval < value_win)
	// 	return eval;

	// Null move pruning
	if (pos.get_ply() && !pv_node && eval >= beta && !in_check && (pos.get_piece_board(BOTH) & ~(pos.get_piece_board(P) | pos.get_piece_board(p))) && !pos.is_last_move_null())
	{
		// Depth calc from stockfish
		int depth_reduction = (1090 + 81 * depth) / 256 + std::min(int(eval - beta) / 205, 3);

		pos.make_null_move();

		int null_score = -negamax(mainThread, pos, -beta, -beta + 1, depth - depth_reduction, tt, uci);

		pos.take_null_move();

		if (null_score >= beta)
		{
			return null_score;
		}
	}

	int legal_moves = 0;

	MoveList ml;
	generate_moves(pos, ml);

	pos.update_follow_pv(ml);

	std::stable_sort(ml.moves, ml.moves + ml.size, compareDescendingMoves);

	for (int i = 0; i < ml.size; i++)
	{
		if (!pos.make_move(ml.moves[i].move, MoveType::ALL))
			continue;

		bool is_quiet_move = is_quiet(ml.moves[i].move);

		if (is_quiet_move && skip_quiet_moves)
		{
			pos.take_move();
			continue;
		}

		if (eval > -value_mate_lower && depth <= 8 && legal_moves >= 4.0 + 4 * depth * depth / 4.5)
		{
			skip_quiet_moves = true;
		}

		legal_moves++;

		// Late Move Reductions
		if (is_quiet_move && depth > 2 && legal_moves > 1)
		{
			reduction = 0.75 + log(depth) * log(legal_moves) / 2.25;
			reduction += !pv_node + in_check;
			reduction = std::min(depth - 1, std::max(reduction, 1));
		}
		else
		{
			reduction = 1;
		}

		// If we hit Late Move Reduction search with reduced depth with modified bounds
		if (reduction != -1)
		{
			score = -negamax(mainThread, pos, -alpha - 1, -alpha, depth - reduction, tt, uci);
		}

		// If our late move reduction returned a value outside of our alpha bound rerun with normal depth
		if ((reduction != -1 && score > alpha) || (reduction == 1 && !(pv_node && legal_moves == 1)))
		{
			score = -negamax(mainThread, pos, -alpha - 1, -alpha, depth - 1, tt, uci);
		}

		// If we are in a pv line and we played a move that beat alpha even on reduced depth,
		// re-search the move with normal bounds and normal depth
		if (pv_node && (legal_moves == 1 || score > alpha))
		{
			score = -negamax(mainThread, pos, -beta, -alpha, depth - 1, tt, uci);
		}

		pos.take_move();

		if (uci.stop_threads)
			return 0;

		// PV move found
		if (score > alpha)
		{
			flag_hash = TranspositionTable::flag_hash_exact;
			if (!is_capture(ml.moves[i].move))
				pos.update_history(ml.moves[i].move, depth);

			alpha = score;

			pos.update_pv(ml.moves[i].move);

			// Fail-hard (failed high)
			if (score >= beta)
			{
				tt.record_hash(pos, depth, beta, TranspositionTable::flag_hash_beta);
				if (!is_capture(ml.moves[i].move))
					pos.update_killer(ml.moves[i].move);
				return beta;
			}
		}
	}

	if (legal_moves == 0)
	{
		if (in_check)
		{
			return mated_in(pos.get_ply());
		}
		return 0;
	}

	tt.record_hash(pos, depth, alpha, flag_hash);

	// failed low
	return alpha;
}

static inline int aspiration(bool mainThread, JACEA::Position &pos, TranspositionTable &tt, UCISettings &uci, int depth, int score)
{
	if (depth == 1)
		return negamax(true, pos, -value_infinite, value_infinite, depth, tt, uci);

	int delta = 25;
	int alpha = std::max(score - delta, -value_infinite);
	int beta = std::min(score + delta, value_infinite);
	for (; !uci.stop; delta += delta / 2)
	{
		score = negamax(mainThread, pos, alpha, beta, depth, tt, uci);

		if (score <= alpha)
		{
			beta = (alpha + beta) / 2;
			alpha = std::max(alpha - delta, -value_infinite);
		}
		else if (score >= beta)
		{
			alpha = (alpha + beta) / 2;
			beta = std::min(beta + delta, value_infinite);
		}
		else
		{
			return score;
		}
	}
	return 0;
}

void start_workers(JACEA::Position &pos, TranspositionTable &tt, UCISettings &uci, int depth, int workers)
{
	int score = 0;
	auto start_time = get_time_ms();
	int real_best = 0;
	pos.init_search();
	uci.completed_iteration = false;
	uci.table_base_hits = 0;
	uci.nodes = 0;

	// Intialize workers
	auto threadPositions = std::vector<JACEA::Position>(workers);
	auto threads = std::vector<std::thread>(workers);
	for (int i = 0; i < workers; i++)
	{
		threadPositions[i] = pos;
	}
	for (int current_depth = 1; current_depth <= depth;)
	{
		pos.follow_pv_true();
		for (int i = 0; i < workers; i++)
		{
			threadPositions[i].follow_pv_true();
		}
		uci.stop_threads = false;
		uci.largest_depth = 0;
		for (int i = 0; i < workers; i++)
		{
			threads[i] = std::thread(aspiration, false, std::ref(threadPositions[i]), std::ref(tt), std::ref(uci), current_depth + i / 2 + 1, score);
		}
		score = aspiration(true, pos, tt, uci, current_depth, score);
		uci.stop_threads = true;
		for (int i = 0; i < workers; i++)
		{
			if (threads[i].joinable())
				threads[i].join();
		}

		if (!uci.stop)
		{
			real_best = pos.get_pv_best();
		}
		else
		{
			break;
		}
		if (score > -value_mate && score < -value_mate_lower)
		{
			printf("info score mate %d depth %d seldepth %d nodes %llu time %llu tbhits %llu pv ", -(score + value_mate + 1) / 2, current_depth, uci.largest_depth, uci.nodes, get_time_ms() - start_time, uci.table_base_hits);
		}
		else if (score > value_mate_lower && score < value_mate)
		{
			printf("info score mate %d depth %d seldepth %d nodes %llu time %llu tbhits %llu pv ", (value_mate - score + 1) / 2, current_depth, uci.largest_depth, uci.nodes, get_time_ms() - start_time, uci.table_base_hits);
		}
		else
		{
			printf("info score cp %d depth %d seldepth %d nodes %llu time %llu tbhits %llu pv ", score, current_depth, uci.largest_depth, uci.nodes, get_time_ms() - start_time, uci.table_base_hits);
		}
		pos.print_pv_line();
		std::cout << std::endl;

		uci.completed_iteration = true;
		current_depth++;
	}
	std::cout << "bestmove " << square_to_coordinate[get_from_square(real_best)] << square_to_coordinate[get_to_square(real_best)];
	if (get_promoted_piece(real_best) != 0)
		std::cout << piece_to_string[get_promoted_piece(real_best)];
	std::cout << std::endl;
}

void JACEA::search(JACEA::Position &pos, TranspositionTable &tt, UCISettings &uci, int depth)
{
	start_workers(pos, tt, uci, depth, 4);
}