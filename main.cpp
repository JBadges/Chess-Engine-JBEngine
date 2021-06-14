#include <cassert>

#include <string>
#include <sstream>
#include <iostream>
#include "random.h"
#include "movegenerator.h"
#include <chrono>
#include <ctime>
#include "eval.h"
#include "search.h"
#include "types.h"
#include "transpositiontable.h"

using namespace JACEA;

unsigned long long perft(JACEA::Position &pos, int depth)
{
	unsigned long long nodes = 0ULL;
	if (depth == 0)
		return 1ULL;

	MoveList ml;
	generate_moves(pos, ml);
	for (int i = 0; i < ml.size; i++)
	{
		if (pos.make_move(ml.moves[i].move, MoveType::ALL))
		{
			nodes += perft(pos, depth - 1);
			pos.take_move();
		}
	}

	return nodes;
}

unsigned long long perft_verbose(JACEA::Position &pos, int depth)
{
	if (depth == 0)
		return 1ULL;

	u64 nodes = 0;
	MoveList ml;
	generate_moves(pos, ml);
	for (int i = 0; i < ml.size; i++)
	{
		u64 node = 0;
		if (pos.make_move(ml.moves[i].move, MoveType::ALL))
		{
			std::cout << square_to_coordinate[get_from_square(ml.moves[i].move)] << square_to_coordinate[get_to_square(ml.moves[i].move)] << " = " << (node = perft(pos, depth - 1)) << std::endl;
			nodes += node;
			pos.take_move();
		}
	}
	std::cout << "Perft " << depth << ": " << nodes << std::endl;
	return nodes;
}

int main(void)
{
	/**
	 * One time initializations
	 */
	init_pawn_attacks();
	init_knight_attacks();
	init_king_attacks();
	init_bishop_mask();
	init_rook_mask();
	init_relevancy_bishop();
	init_relevancy_rook();
	init_magic_numbers();
	init_bishop_magic_attack();
	init_rook_magic_attack();
	init_zobrist_keys();
	init_pst();
	init_mvv_lva();
	nnue_init("nn-eba324f53044.nnue");

	JACEA::Position pos;
	std::vector<TTEntry> transposition_table(hash_table_size, {0, 0, 0, 0, 0});

	std::cout << "uciok" << std::endl;

	while (true)
	{
		std::string line, token;
		std::getline(std::cin, line);
		std::istringstream tokenizer{line};
		tokenizer >> token;

		if (token == "ucinewgame")
		{
			clear_table(transposition_table, hash_table_size);
		}
		else if (token == "uci")
		{
			std::cout << "id name JACEA 1.0" << std::endl;
			std::cout << "id author Jackson (JBadges) Brajer" << std::endl;
			std::cout << "uciok" << std::endl;
		}
		else if (token == "isready")
		{
			std::cout << "readyok" << std::endl;
		}
		else if (token == "position")
		{
			parse_position(pos, tokenizer);
		}
		else if (token == "go")
		{
			parse_go(pos, transposition_table, tokenizer);
		}
		else if (token == "quit")
		{
			break;
		}
		else if (token == "p")
		{
			pos.print();
			std::cout << "Turn (0=w,1=b): " << pos.get_side() << std::endl;
			std::cout << "Evaluation: " << std::dec << evaluation(pos) << std::endl;
		}
	}
	return 0;
}