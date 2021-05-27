#include <cassert>

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

unsigned long long perft(Position &pos, int depth)
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

unsigned long long perft_verbose(Position &pos, int depth)
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

	Position pos;
	std::vector<TTEntry> transposition_table(hash_table_size, {0, 0, 0, 0, 0});
	std::cout << "uciok" << std::endl;

	while (true)
	{
		std::string line;
		std::getline(std::cin, line);
		if (line.substr(0, 3) == "uci")
		{
			std::cout << "id name JACEA 1.0" << std::endl;
			std::cout << "id author Jackson (JBadges) Brajer" << std::endl;
			std::cout << "uciok" << std::endl;
		}
		else if (line.substr(0, 7) == "isready")
		{
			std::cout << "readyok" << std::endl;
		}
		else if (line.substr(0, 8) == "position")
		{
			parse_position(pos, line);
		}
		else if (line.substr(0, 2) == "go")
		{
			parse_go(pos, transposition_table, line);
		}
		else if (line.substr(0, 4) == "quit")
		{
			break;
		}
		else if (line.substr(0, 4) == "ucinewgame")
		{
			parse_position(pos, "position startpos");
			clear_table(transposition_table, hash_table_size);
		}
		else if (line.substr(0, 1) == "p")
		{
			pos.print();
			std::cout << "Turn (0=w,1=b): " << pos.get_side() << std::endl;
			std::cout << "Evaluation: " << std::dec << evaluation(pos) << std::endl;
		}
	}
	return 0;
}