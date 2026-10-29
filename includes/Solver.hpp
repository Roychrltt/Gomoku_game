#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <vector>
#include "Position.hpp"

# define RED "\033[31m"
# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define BLUE "\033[34m"
# define MAGENTA "\033[35m"
# define CYAN "\033[36m"
# define GRAY "\033[90m"
# define BOLD "\033[1m"
# define UNDER "\033[4m"
# define BLINK "\033[5m"
# define ERASE = "\033[2K\r"
# define RESET "\033[0m"

class Solver {
	public:
		enum Cell
		{
			EMPTY = 0,
			BLACK = 1,
			WHITE = 2
		};

		int _rootPlayer = 1;
		static constexpr unsigned int SIZE = 19;
		static constexpr unsigned int CELLS = SIZE * SIZE;
		static constexpr unsigned int MIN_DEPTH = 1;
		static constexpr unsigned int MAX_DEPTH = 10;
		static constexpr int CAPTURE_TO_WIN = 10;

		static constexpr int WIN = 100000000;
		static constexpr int INF = 2000000000;
		std::vector<std::vector<uint8_t>> _board;

		Solver() = default;
		~Solver() = default;

		Position solve(const std::vector<std::vector<uint8_t>>& board, int turn, const std::array<int, 2>& captureStone);

		unsigned int getLastDepth() const;
		int _evaluate() const;
		unsigned long long getLastNodes() const;
		int getLastScore() const;
		std::string getLastPV() const;

	private:
		struct Move {
			int x;
			int y;
			int score;
		};

		struct Undo {
			int x;
			int y;
			int capturex1;
			int capturey1;
			int capturex2;
			int capturey2;
			bool captured;
			std::array<int, 2> capturesBefore;
		};

		struct TableEntry {
			int score;
			int move;
			unsigned int depth;
			uint8_t bound;
		};

		static constexpr int TIME_LIMIT_MS = 500;
		static constexpr int MAX_MOVES = 24;

		static constexpr int FIVE = 1000000;
		static constexpr int OPEN_FOUR = 50000;
		static constexpr int FOUR = 10000;
		static constexpr int OPEN_THREE = 5000;
		static constexpr int THREE = 1000;
		static constexpr int OPEN_TWO = 50;
		static constexpr int TWO = 10;
		static constexpr int STONE = 1;
		static constexpr int CAPTURE = 5000;
		static constexpr int CAPTURE_PROGRESS = 1000;

		//std::vector<std::vector<uint8_t>> _board;
		std::array<int, 2> _captures{0, 0};
		//int _rootPlayer = 1;

		unsigned long long _nodes = 0;
		bool _aborted = false;
		std::chrono::steady_clock::time_point _start;

		unsigned int _lastDepth = 0;
		unsigned long long _lastNodes = 0;
		int _lastScore = 0;
		std::string _lastPV;

		int _pv[MAX_DEPTH + 1]{};
		std::vector<Undo> _undo;
		std::unordered_map<uint64_t, TableEntry> _table;

		static bool _onBoard(int x, int y);
		void _load(const std::vector<std::vector<uint8_t>>& board, int turn, const std::array<int, 2>& captureStone);

		std::vector<Move> _generateMoves(int player);
		int _moveScore(int x, int y, int player);

		bool _play(const int x, const int y, int player);
		void _unplay();
		int _scorePosition(int x, int y, int player) const;
		int _captureScore(int player) const;
		uint64_t _hash(int player) const;
		int _minmax(unsigned int depth, int alpha, int beta, int player);
		bool _timeUp();
		void _buildPV();
};

#endif
