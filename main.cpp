#include <cassert>

#include <iostream>
#include <bitset>
#include "attacks.h"
#include "random.h"
#include "position.h"
#include "move.h"

using namespace JACEA;

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
	p.init_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	p.print();
	p.make_move(create_move(d2, d4, 0, 0), MoveType::ALL);
	p.print();
	p.make_move(create_move(d7, d5, 0, 0), MoveType::ALL);
	p.print();
	return 0;
}