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
	p.init_from_fen("rn2kbnr/pbppqppp/1p6/5p2/3P4/1PN2P1N/PBP1P1PP/R2QKB1R w KQkq - 0 1");
	p.print();
	return 0;
}