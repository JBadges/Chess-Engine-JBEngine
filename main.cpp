#include <cassert>

#include <iostream>
#include <bitset>
#include "attacks.h"
#include "random.h"
#include "position.h"
#include "move.h"
#include "movegenerator.h"
#include <chrono>
#include <ctime>

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

	// Some computation here

	Position p;
	p.init_from_fen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
	auto start = std::chrono::system_clock::now();
	u64 nodes = perft_verbose(p, 7);
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);

	std::cout << "nodes: \t" << nodes << "\n";
	std::cout << "time: \t" << elapsed_seconds.count() << "\n";
	std::cout << "\t" << std::fixed << nodes / elapsed_seconds.count() << "n/s\n";
	return 0;
}