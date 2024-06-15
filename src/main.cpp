#include <cassert>
#include <thread>
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
#include "tbprobe.h"
#include <filesystem>
#include "utility.h"
#include <future>

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
	nnue_init(std::filesystem::absolute(NNUE_FILE_PATH).generic_string().c_str());
    tb_init(std::filesystem::absolute(SYZYGY_PATH).generic_string().c_str());

	const size_t default_hash_size_mb = 64;

	JACEA::Position pos;
	JACEA::UCISettings uci_settings;
	std::future<void> search_future;
	TranspositionTable transposition_table;
	transposition_table.resize_table_mb(default_hash_size_mb);
	transposition_table.clear_table();
	
	std::istringstream startpos_tokenizer{"startpos"};
	parse_position(pos, startpos_tokenizer);

	std::cout << "uciok" << std::endl;

	while (true)
	{
		std::string line, token;
		std::getline(std::cin, line);
		std::istringstream tokenizer{line};
		while (tokenizer >> token) {
			if (token == "setoption")
			{
				parse_setoption(transposition_table, tokenizer);
			}
			else if (token == "debug") 
			{
				std::cerr << "Debugging currently not supported" << std::endl;
			} 
			else if (token == "stop")
			{
				uci_settings.stop = true;
				if (search_future.valid())
				{
					search_future.wait();
				}
			}
			else if (token == "register")
			{
				std::cerr << "Registration currently not supported" << std::endl;
			}
			else if (token == "ponderhit")
			{
				std::cerr << "Pondering currently not supported" << std::endl;
			}
			else if (token == "ucinewgame")
			{
				transposition_table.clear_table();
			}
			else if (token == "uci")
			{
				std::cout << "id name JACEA 1.0" << std::endl;
				std::cout << "id author Jackson (JBadges) Brajer" << std::endl;
				std::cout << std::endl;
				std::cout << "option name Hash type spin default " << default_hash_size_mb << " min 0" << std::endl;
				std::cout << "option name SyzygyPath type string default <empty>" << std::endl;
				std::cout << "option name NNUEPath type string default <empty>" << std::endl;
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
				if (search_future.valid())
				{
					search_future.wait();
				}

				// Have to make a copy of the line since line will be overwritten on next loop as this thread does not halt the execution
				std::string tokenizer_state = tokenizer.str();
				search_future = std::async(std::launch::async, [&, tokenizer_state]() mutable {
					std::istringstream local_tokenizer{ tokenizer_state };
					parse_go(pos, transposition_table, uci_settings, local_tokenizer);
				});
			}
			else if (token == "quit")
			{
				uci_settings.stop = true;
				if (search_future.valid())
				{
					search_future.wait();
				}
				return 0;
			}
			else if (token == "p")
			{
				pos.print();
				std::cout << "Turn (0=w,1=b): " << pos.get_side() << std::endl;
				std::cout << "Evaluation: " << std::dec << evaluation(pos) << std::endl;
				unsigned res = tb_probe_root(bswap64(pos.get_occupancy_board(WHITE)),
											bswap64(pos.get_occupancy_board(BLACK)),
											bswap64(pos.get_piece_board(k) | pos.get_piece_board(K)),
											bswap64(pos.get_piece_board(q) | pos.get_piece_board(Q)),
											bswap64(pos.get_piece_board(r) | pos.get_piece_board(R)),
											bswap64(pos.get_piece_board(b) | pos.get_piece_board(B)),
											bswap64(pos.get_piece_board(n) | pos.get_piece_board(N)),
											bswap64(pos.get_piece_board(p) | pos.get_piece_board(P)),
											pos.get_fifty(),
											pos.get_castling_perms(),
											pos.get_enpassant_square(),
											pos.get_side() ^ 1,
											nullptr);
				if (res != TB_RESULT_FAILED)
				{
					unsigned wdl = TB_GET_WDL(res);
					static const char *wdl_to_str[5] =
						{
							"0-1",
							"1/2-1/2",
							"1/2-1/2",
							"1/2-1/2",
							"1-0"};
					std::cout << "TB Probe: " << wdl_to_str[wdl] << std::endl;
				}
			}
			else if (token == "perft")
			{
				int perft_depth;
				if (!(tokenizer >> token))
				{
					perft_depth = 6;
				}
				else
				{
					perft_depth = std::stoi(token);
				}
				auto start_time = get_time_ms();
				unsigned long long nodes = perft_verbose(pos, perft_depth);
				auto duration = get_time_ms() - start_time;
				std::cout << "Nodes \t\t: " << nodes << std::endl;
				std::cout << "Time (s) \t: " << duration / 1000.0 << std::endl;
				std::cout << "Nodes/s \t: " << std::fixed << nodes / (duration / 1000.0) << std::endl;
			}
		}
	}
	return 0;
}