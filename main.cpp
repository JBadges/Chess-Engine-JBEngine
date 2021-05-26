#include <cassert>

#include <iostream>
#include "random.h"
#include "movegenerator.h"
#include <chrono>
#include <ctime>
#include "search.h"

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

	Position p;
	p.init_from_fen("r2qkb1r/2p2ppp/p1n1b3/1p1Np3/3P4/1B3N2/PPP2PPP/R1BQK2R w KQkq - 0 1");
	search(p, 7);
	return 0;
}