#include <cassert>

#include <iostream>
#include <bitset>
#include "attacks.h"
#include "types.h"
#include "random.h"

using namespace JACEA;

int main(void)
{
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

	Bitboard b = 0ull;
	set_bit(b, e5);
	set_bit(b, a1);
	set_bit(b, h5);
	set_bit(b, a2);
	set_bit(b, b6);
	print_bitboard(b);
	print_bitboard(get_queen_attacks(e3, b));
	return 0;
}