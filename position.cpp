#include <assert.h>
#include "position.h"
#include "random.h"
#include <iostream>

using namespace JACEA;

u64 JACEA::piece_position_key[13][64];
u64 JACEA::side_key;
u64 JACEA::castle_perm_key[16];
int JACEA::square_to_castle_perm[64] = {
	(15 & ~bq), 15, 15, 15, (15 & ~bq & ~bk), 15, 15, (15 & ~bk),
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	(15 & ~wq), 15, 15, 15, (15 & ~wq & ~wk), 15, 15, (15 & ~wk)};

void JACEA::init_zobrist_keys()
{
	for (Piece p = 0; p < 13; p++)
	{
		for (Square square = 0; square < 64; square++)
		{
			piece_position_key[p][square] = random_u64();
		}
	}
	for (int i = 0; i < 16; i++)
	{
		castle_perm_key[i] = random_u64();
	}
	side_key = random_u64();
}

void JACEA::Position::check() const
{
	// This function should only be run if in debug mode
#ifndef NDEBUG
	int white_val = 0;
	int black_val = 0;

	for (Square sq = 0; sq < 64; sq++)
	{
		if (mailbox[sq] == None)
			continue;
		assert(get_bit(piece_boards[mailbox[sq]], sq));
		if (color_from_piece[mailbox[sq]] == WHITE)
			white_val += piece_to_value[mailbox[sq]];
		else
			black_val += piece_to_value[mailbox[sq]];
	}
	assert(white_val == white_material);
	assert(black_val == black_material);

	Bitboard whitebb = 0ull;
	Bitboard blackbb = 0ull;
	Bitboard bothbb = 0ull;
	for (int piece = P; piece <= K; piece++)
	{
		whitebb |= piece_boards[piece];
	}

	for (int piece = p; piece <= k; piece++)
	{
		blackbb |= piece_boards[piece];
	}

	bothbb = whitebb | blackbb;

	assert(pop_count(piece_boards[p]) <= 8);
	assert(pop_count(piece_boards[P]) <= 8);
	assert(whitebb == occupancy[WHITE]);
	assert(blackbb == occupancy[BLACK]);
	assert(bothbb == occupancy[BOTH]);
	assert(zobrist_key == generate_zobrist_key());
	assert(pop_count(piece_boards[K]) == 1);
	assert(pop_count(piece_boards[k]) == 1);
#endif
}

u64 JACEA::Position::generate_zobrist_key() const
{
	u64 key = 0ULL;

	for (Square square = 0; square < 64; square++)
	{
		if (mailbox[square] == None)
			continue;
		key ^= piece_position_key[mailbox[square]][square];
	}

	if (side == WHITE)
		key ^= side_key;

	if (en_passant != no_sq)
		key ^= piece_position_key[None][en_passant];

	key ^= castle_perm_key[castling];

	return key;
}

JACEA::Position::Position()
{
	reset();
}

Position &JACEA::Position::operator=(const Position &rhs)
{
	if (this == &rhs)
		return *this;

	zobrist_key = rhs.zobrist_key;
	for (int i = 0; i < 12; i++)
		piece_boards[i] = rhs.piece_boards[i];
	for (int i = 0; i < 3; i++)
		occupancy[i] = rhs.occupancy[i];
	for (int i = 0; i < 64; i++)
		mailbox[i] = rhs.mailbox[i];
	side = rhs.side;
	en_passant = rhs.en_passant;
	castling = rhs.castling;
	ply = rhs.ply;
	rule50 = rhs.rule50;

	for (int i = 0; i < max_game_ply; i++)
		history[i] = rhs.history[i];
	history_size = rhs.history_size;

	white_material = rhs.white_material;
	black_material = rhs.black_material;

	for (int i = 0; i < max_game_depth; i++)
	{
		killer_moves[0][i] = rhs.killer_moves[0][i];
		killer_moves[1][i] = rhs.killer_moves[1][i];
	}
	for (int i = 0; i < 12; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			history_moves[i][j] = rhs.history_moves[i][j];
		}
	}

	for (int i = 0; i < max_game_depth; i++)
		pv_length[i] = rhs.pv_length[i];
	for (int i = 0; i < max_game_depth; i++)
	{
		for (int j = 0; j < max_game_depth; j++)
		{
			pv_table[i][j] = rhs.pv_table[i][j];
		}
	}

	follow_pv = rhs.follow_pv;
	score_pv = rhs.score_pv;

	return *this;
}

void JACEA::Position::reset()
{
	zobrist_key = 0ULL;
	for (int i = 0; i < 12; i++)
		piece_boards[i] = 0ULL;
	for (int i = 0; i < 3; i++)
		occupancy[i] = 0ULL;
	for (int i = 0; i < 64; i++)
		mailbox[i] = None;

	side = WHITE;
	en_passant = no_sq;
	castling = 0;
	ply = 0;
	rule50 = 0;

	history_size = 0;

	white_material = 0;
	black_material = 0;

	zobrist_key = generate_zobrist_key();
}

void JACEA::Position::init_from_fen(std::string fen)
{
	// Step 1: Clear position to ensure we are starting from the same object each time
	reset();

	auto c = fen.c_str();

	// Step 2: init piece bitboards from fen
	for (auto square = 0; square < 64; c++)
	{
		// If the current char is a number
		if ('0' <= *c && *c <= '9')
		{
			// Assuming the fen string is formatted properly, the file will never be overrun
			square += *c - '0';
		}
		// if A-z we need to add a piece to a specific bitboard
		else if (('A' <= *c && *c <= 'Z') || ('a' <= *c && *c <= 'z'))
		{
			assert(*c == 'P' || *c == 'N' || *c == 'B' || *c == 'R' || *c == 'Q' || *c == 'K' ||
				   *c == 'p' || *c == 'n' || *c == 'b' || *c == 'r' || *c == 'q' || *c == 'k');

			const int piece = char_to_piece(*c);
			add_piece(piece, square);
			square++;
		}
	}

	// Step 3: init side, castling, enPassant
	while (*c == ' ')
		c++;

	side = (*c++ == 'w') ? WHITE : BLACK;

	while (*++c != ' ')
	{
		switch (*c)
		{
		case 'K':
			castling |= wk;
			break;
		case 'Q':
			castling |= wq;
			break;
		case 'k':
			castling |= bk;
			break;
		case 'q':
			castling |= bq;
			break;
		case '-':
			castling = 0;
			break;
		default:
			assert(false);
		}
	}

	en_passant = no_sq;
	if (*++c != '-')
	{
		const int file = *c++ - 'a';
		const int rank = 8 - (*c++ - '0');
		const int square = rank * 8 + file;
		en_passant = square;
	}

	zobrist_key = generate_zobrist_key();

	// For search
	ply = 0;

	check();
}

bool JACEA::Position::make_move(Move move, const MoveType mt)
{
	if (mt == MoveType::CAPTURES)
	{
		if (!is_capture(move))
			return false;
		return make_move(move, MoveType::ALL);
	}

	history[history_size].castling = castling;
	history[history_size].en_passant = en_passant;
	history[history_size].plies = ply;
	history[history_size].rule50 = rule50;
	history[history_size].move = move;
	history[history_size].key = zobrist_key;

	// Remove enpassant and castle from key in case of change
	if (en_passant != no_sq)
		zobrist_key ^= piece_position_key[12][en_passant];
	zobrist_key ^= castle_perm_key[castling];

	ply++;
	rule50++;
	en_passant = no_sq;

	const int from_square = get_from_square(move);
	const int to_square = get_to_square(move);
	const int piece = mailbox[from_square];
	const int promoted_piece = get_promoted_piece(move);
	const int is_capture_move = is_capture(move);
	const int is_doublepawnpush_move = is_doublepawnpush(move);
	const int is_castle_move = is_castle(move);
	const int is_enpassant_move = is_enpassant(move);

	if (mailbox[from_square] == P || mailbox[from_square] == p)
	{
		rule50 = 0;
	}

	// Step 1: If its a capture move, store it and remove the captured piece
	if (is_capture_move)
	{
		assert(mailbox[to_square] != None);
		rule50 = 0;
		history[history_size].captured_piece = mailbox[to_square];
		take_piece(to_square);
	}

	// Step 2: Remove enpassant capture
	if (is_enpassant_move)
	{
		if (side == WHITE)
		{
			take_piece(to_square + 8);
			history[history_size].captured_piece = p;
		}
		else
		{
			take_piece(to_square - 8);
			history[history_size].captured_piece = P;
		}
	}
	// Step 3: Move piece to target square
	move_piece(from_square, to_square);

	// Step 4: If move was a castle move rook to correct position
	if (is_castle_move)
	{
		switch (to_square)
		{
		case g1:
			move_piece(h1, f1);
			break;
		case c1:
			move_piece(a1, d1);
			break;
		case g8:
			move_piece(h8, f8);
			break;
		case c8:
			move_piece(a8, d8);
			break;
		}
	}

	// Step 5: If its a promotion remove pawn from 8th/1st rank and add promoted piece
	if (piece == P && a8 <= to_square && to_square <= h8)
	{
		take_piece(to_square);
		add_piece(promoted_piece, to_square);
	}
	else if (piece == p && a1 <= to_square && to_square <= h1)
	{
		take_piece(to_square);
		add_piece(promoted_piece, to_square);
	}

	// Step 6: Update enpassant square
	if (is_doublepawnpush_move)
	{
		if (side == WHITE)
		{
			en_passant = to_square + 8;
		}
		else
		{
			en_passant = to_square - 8;
		}
		zobrist_key ^= piece_position_key[12][en_passant];
	}

	// Step 7: Castle perms and side swap
	castling &= square_to_castle_perm[from_square];
	castling &= square_to_castle_perm[to_square];
	side ^= 1;
	zobrist_key ^= side_key;
	zobrist_key ^= castle_perm_key[castling];

	history_size++;

	if (side == WHITE)
	{
		if (is_square_attacked(side, get_firstlsb_index(piece_boards[k])))
		{
			take_move();
			check();
			return false;
		}
	}
	else
	{
		if (is_square_attacked(side, get_firstlsb_index(piece_boards[K])))
		{
			take_move();
			check();
			return false;
		}
	}
	check();
	return true;
}

void JACEA::Position::take_move()
{
	assert(history[history_size - 1].move);
	assert(history_size);
	history_size--;

	if (en_passant != no_sq)
		zobrist_key ^= piece_position_key[12][en_passant];
	zobrist_key ^= castle_perm_key[castling];

	castling = history[history_size].castling;
	en_passant = history[history_size].en_passant;
	ply = history[history_size].plies;
	rule50 = history[history_size].rule50;

	if (en_passant != no_sq)
		zobrist_key ^= piece_position_key[12][en_passant];
	zobrist_key ^= castle_perm_key[castling];

	const int captured_piece = history[history_size].captured_piece;

	const int move = history[history_size].move;
	const int from_square = get_from_square(move);
	const int to_square = get_to_square(move);
	const int promoted_piece = get_promoted_piece(move);
	const bool is_capture_move = is_capture(move);
	const bool is_castle_move = is_castle(move);
	const bool is_enpassant_move = is_enpassant(move);

	// Step 1: If the move was a promotion, turn the promoted piece back into a pawn
	if (promoted_piece != 0 && a8 <= to_square && to_square <= h8)
	{
		take_piece(to_square);
		add_piece(P, to_square);
	}
	else if (promoted_piece != 0 && a1 <= to_square && to_square <= h1)
	{
		take_piece(to_square);
		add_piece(p, to_square);
	}

	// Step 2: Move piece(s) back to original square
	move_piece(to_square, from_square);

	// If the move was a castle, depending on where the king moved to will determine where to move the rook
	if (is_castle_move)
	{
		switch (to_square)
		{
		case g1:
			move_piece(f1, h1);
			break;
		case c1:
			move_piece(d1, a1);
			break;
		case g8:
			move_piece(f8, h8);
			break;
		case c8:
			move_piece(d8, a8);
			break;
		default:
			assert(false);
		}
	}

	// Step 3: Add the captured piece back to its square
	if (is_enpassant_move)
	{
		if (side == WHITE)
		{
			add_piece(P, to_square - 8);
		}
		else
		{
			add_piece(p, to_square + 8);
		}
	}
	else if (is_capture_move)
	{
		add_piece(captured_piece, to_square);
	}

	// Step 4: Change back side
	side ^= 1;
	zobrist_key ^= side_key;
	check();
}

void JACEA::Position::make_null_move()
{
	ply++;
	history[history_size].move = 0;
	history[history_size].rule50 = rule50;
	history[history_size].en_passant = en_passant;
	history[history_size].castling = castling;
	history[history_size].key = en_passant;

	if (en_passant != no_sq)
		zobrist_key ^= piece_position_key[12][en_passant];

	en_passant = no_sq;
	history_size++;
	side ^= 1;
	zobrist_key ^= side_key;
}
void JACEA::Position::take_null_move()
{
	history_size--;

	ply--;

	castling = history[history_size].castling;
	en_passant = history[history_size].en_passant;
	rule50 = history[history_size].rule50;

	if (en_passant != no_sq)
		zobrist_key ^= piece_position_key[12][en_passant];
	side ^= 1;
	zobrist_key ^= side_key;
}

void JACEA::Position::print() const
{
	std::cout << std::endl;
	for (int rank = 0; rank < 8; rank++)
	{
		for (int file = 0; file < 8; file++)
		{
			const int square = rank * 8 + file;

			if (!file)
				printf(" %d ", 8 - rank);

			std::cout << " " << piece_to_string[mailbox[square]] << " ";
		}
		printf("\n");
	}
	printf("    a  b  c  d  e  f  g  h\n\n");
}