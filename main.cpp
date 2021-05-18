#include <cassert>

#include "bitboard.h"

int main(void)
{
	Bitboard b = 0ull;
	set_bit(b, a2);
	set_bit(b, b2);
	set_bit(b, c2);
	set_bit(b, d2);
	set_bit(b, e2);
	set_bit(b, f2);
	set_bit(b, g2);
	set_bit(b, h2);

	print_bitboard(b);

	b = shift<DOWN_RIGHT>(b);

	print_bitboard(b);

	return 0;
}