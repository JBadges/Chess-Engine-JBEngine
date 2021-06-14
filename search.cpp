#include "search.h"
#include "eval.h"
#include "movegenerator.h"
#include "utility.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include "random.h"

using namespace JACEA;

static inline bool compareDescendingMoves(const ScoredMove &a, const ScoredMove &b)
{
	return a.score > b.score;
}

static inline void update_stop(UCISettings &uci)
{
	uci.stop = uci.completed_iteration && get_time_ms() > uci.time_to_stop;
}

static inline int quiesence(bool mainThread, JACEA::Position &pos, int alpha, int beta, UCISettings &uci)
{
	if (mainThread)
		uci.nodes++;

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

static inline int negamax(bool mainThread, JACEA::Position &pos, int alpha, int beta, int depth, std::vector<TTEntry> &tt, UCISettings &uci)
{
	if (uci.stop_threads)
		return 0;
	pos.update_current_pv_length();

	if (mainThread)
		uci.nodes++;

	if (uci.nodes & 15000) // ~ each ms
	{
		update_stop(uci);
	}

	if (uci.stop)
	{
		return 0;
	}

	// Draw
	if (pos.get_ply() != 0 && pos.draw())
	{
		return 0;
	}

	if (pos.get_ply() >= max_game_ply)
	{
		return evaluation(pos);
	}

	if (depth <= 0)
	{
		return quiesence(mainThread, pos, alpha, beta, uci);
	}

	bool pv_node = (beta - alpha) > 1;
	int eval = evaluation(pos);
	int score;
	int flag_hash = flag_hash_alpha;

	// Transposition table lookup
	if (pos.get_ply() && ((score = read_hash_entry(pos, tt, alpha, beta, depth)) != no_hash) && !pv_node && pos.get_fifty() < 90)
	{
		return score;
	}

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

	// Futility pruning
	if (pos.get_ply() && !pv_node && depth < 9 && eval - (200 * depth) >= beta && eval < value_win)
		return eval;

	bool in_check = pos.is_square_attacked(pos.get_side() ^ 1, (pos.get_side() == WHITE) ? get_firstlsb_index(pos.get_piece_board(K)) : get_firstlsb_index(pos.get_piece_board(k)));

	// Null move pruning
	if (!pv_node && eval >= beta && !in_check)
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

	if (in_check)
		depth++;

	int legal_moves = 0;

	MoveList ml;
	generate_moves(pos, ml);

	pos.update_follow_pv(ml);

	std::stable_sort(ml.moves, ml.moves + ml.size, compareDescendingMoves);

	int moves_searched = 0;

	for (int i = 0; i < ml.size; i++)
	{
		if (!pos.make_move(ml.moves[i].move, MoveType::ALL))
			continue;

		legal_moves++;
		// We want to run a full search
		if (moves_searched == 0)
		{
			score = -negamax(mainThread, pos, -beta, -alpha, depth - 1, tt, uci);
		}
		// Late Move Reduction
		else
		{
			if (depth >= 3 && moves_searched >= 1 + 2 * pv_node && !in_check && !is_capture(ml.moves[i].move) && !get_promoted_piece(ml.moves[i].move))
			{
				score = -negamax(mainThread, pos, -alpha - 1, -alpha, depth - 2, tt, uci);
			}
			else
			{
				score = alpha + 1; // Hack to force full depth search
			}
			// PVS
			if (score > alpha)
			{
				score = -negamax(mainThread, pos, -alpha - 1, -alpha, depth - 1, tt, uci);

				if (score > alpha && score < beta)
				{
					score = -negamax(mainThread, pos, -beta, -alpha, depth - 1, tt, uci);
				}
			}
		}

		pos.take_move();

		moves_searched++;

		if (uci.stop_threads)
			return 0;

		// PV move found
		if (score > alpha)
		{
			flag_hash = flag_hash_exact;
			if (!is_capture(ml.moves[i].move))
				pos.update_history(ml.moves[i].move, depth);

			alpha = score;

			pos.update_pv(ml.moves[i].move);

			// Fail-hard (failed high)
			if (score >= beta)
			{
				record_hash(pos, tt, depth, beta, flag_hash_beta);
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

	// If we only have one legal move on our root node end iterative deepening
	if (pos.get_ply() == 0 && legal_moves == 1)
	{
		uci.end_early = true;
	}

	record_hash(pos, tt, depth, alpha, flag_hash);

	// failed low
	return alpha;
}

static inline int aspiration(bool mainThread, JACEA::Position &pos, std::vector<TTEntry> &tt, UCISettings &uci, int depth, int score)
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

void JACEA::search(JACEA::Position &pos, std::vector<TTEntry> &tt, UCISettings &uci, int depth)
{
	auto start_time = get_time_ms();
	int real_best = 0;
	pos.init_search();
	uci.completed_iteration = false;

	const int workers = 3;
	JACEA::Position *threadPositions = new JACEA::Position[workers];
	std::thread threads[workers];
	int score = 0;
	for (int i = 0; i < workers; i++)
	{
		threadPositions[i] = pos;
	}
	for (int current_depth = 1; current_depth <= depth;)
	{
		pos.follow_pv_true();
		uci.stop_threads = false;
		for (int i = 0; i < workers; i++)
		{
			threads[i] = std::thread(aspiration, false, std::ref(threadPositions[i]), std::ref(tt), std::ref(uci), current_depth + i / 2 + 1, score);
		}
		score = aspiration(true, pos, tt, uci, current_depth, score);
		uci.stop_threads = true;
		for (int i = 0; i < workers; i++)
		{
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
			printf("info score mate %d depth %d nodes %llu time %llu pv ", -(score + value_mate) / 2 - 1, current_depth, uci.nodes, get_time_ms() - start_time);
			uci.end_early = true;
		}
		else if (score > value_mate_lower && score < value_mate)
		{
			printf("info score mate %d depth %d nodes %llu time %llu pv ", (value_mate - score) / 2 + 1, current_depth, uci.nodes, get_time_ms() - start_time);
			uci.end_early = true;
		}
		else
		{
			printf("info score cp %d depth %d nodes %llu time %llu pv ", score, current_depth, uci.nodes, get_time_ms() - start_time);
		}
		pos.print_pv_line();
		std::cout << std::endl;

		uci.completed_iteration = true;
		current_depth++;
		if (uci.end_early)
		{
			break;
		}
	}
	std::cout << "bestmove " << square_to_coordinate[get_from_square(real_best)] << square_to_coordinate[get_to_square(real_best)];
	if (get_promoted_piece(real_best) != 0)
		std::cout << piece_to_string[get_promoted_piece(real_best)];
	std::cout << '\n';
}