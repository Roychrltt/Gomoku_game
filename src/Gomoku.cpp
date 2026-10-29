# include "Gomoku.hpp"
# include <cmath>
# include "Solver.hpp"

Gomoku::Gomoku() : _board(SIZE, std::vector<uint8_t>(SIZE)), _ui(UI()), _turn(1), _winner(0), _ai(false), _sugg(false), _suggPos(Position({-1, -1})), _captureStone({0, 0}), _winnerRecord({0, 0})
{
	_solver = Solver();
}

Gomoku::~Gomoku(){}

void Gomoku::printBoardCount() const
{
	int whitecnt = 0, blackcnt = 0;
	for (int i = 0; i < 19; i++)
	{
		for (int j = 0; j < 19; j++)
		{
			if (_board[i][j] == WHITE) whitecnt++;
			else if (_board[i][j] == BLACK) blackcnt++;
		}
	}
	std::cout << "white: " << whitecnt << "   black: " << blackcnt << std::endl;		
}

void Gomoku::_reset(){
	_turn = 1;
	_winner = 0;
	_suggPos = Position({-1, -1});
	_moveHistory.clear();
	_moveHistory.push_back(History{-1, -1, -1});
	_board = std::vector<std::vector<uint8_t>>(SIZE, std::vector<uint8_t>(SIZE));
	_captureStone = {0, 0};
	solveTime.clear();
}

void Gomoku::useAI(bool ai){
	_ai = ai;
}

int Gomoku::checkWinning(const std::vector<std::vector<uint8_t>>& board, int player, const std::array<int, 2>& captureStone)
{
	if (captureStone[0] >= Solver::CAPTURE_TO_WIN) return 1;
	if (captureStone[1] >= Solver::CAPTURE_TO_WIN) return 2;

	int opponent = 3 ^ player;
	static const int dirs[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
	for (int x = 0; x < static_cast<int>(SIZE); ++x)
	{
		for (int y = 0; y < static_cast<int>(SIZE); ++y)
		{
			if (board[x][y] != opponent) continue;

			for (const auto& dir : dirs)
			{
				int endX = x + 4 * dir[0];
				int endY = y + 4 * dir[1];

				if (!isOnBoard(endX, endY)) continue;

				bool win = true;

				for (int i = 1; i < 5; ++i)
				{
					int nx = x + i * dir[0];
					int ny = y + i * dir[1];

					if (board[nx][ny] != opponent)
					{
						win = false;
						break;
					}
				}

				if (win) return opponent;
			}
		}
	}
	return 0;
}

bool Gomoku::capture(std::vector<std::vector<uint8_t>>& board, int x, int y, int turn, std::array<int, 2>& captureStone, std::array<int, 2>* captured)
{
	if (captured != nullptr) *captured = {-1, -1};

	const int directions[8][2] = {{1, 0}, {-1, 0},{0, 1}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};
	for (const auto& dir : directions)
	{
		int dx = dir[0], dy = dir[1];
		int x3 = x + 3 * dx, y3 = y + 3 * dy;

		if (!isOnBoard(x3, y3)) continue;

		if (board[x3][y3] == turn && board[x + dx][y + dy] == (3 ^ turn) && board[x + 2 * dx][y + 2 * dy] == (3 ^ turn))
		{
			if (captured != nullptr) *captured = {x + dx, y + dy};

			board[x + dx][y + dy] = 0;
			board[x + 2 * dx][y + 2 * dy] = 0;
			captureStone[turn - 1] += 2;
			return true;
		}
	}

	return false;
}

bool Gomoku::isOnBoard(int x, int y)
{
	return x >= 0 && x < SIZE && y >= 0 && y < SIZE;
}

bool Gomoku::checkDoubleFreeThree(const std::vector<std::vector<uint8_t>>& board, int turn, int lastX, int lastY)
{
	static const int dirs[8][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0},{-1, -1}, {1, 1}, {1, -1}, {-1, 1}};

	uint8_t threeCount = 0;
	uint8_t opponentColor = turn == 1 ? 2 : 1;

	for (int i = 0; i < 4; ++i) {
		uint8_t curCount = 0;
		int dx1 = dirs[i * 2][0], dy1 = dirs[i * 2][1];
		int dx2 = dirs[i * 2 + 1][0], dy2 = dirs[i * 2 + 1][1];
		int x1 = lastX + dx1, y1 = lastY + dy1;
		int x2 = lastX + dx2, y2 = lastY + dy2;

		if (isOnBoard(x1, y1) && board[x1][y1] != opponentColor && isOnBoard(x2, y2) && board[x2][y2] != opponentColor)
			curCount = 1;
		else continue;

		int distance = 0;
		while (isOnBoard(x1 + dx1, y1 + dy1) && board[x1 + dx1][y1 + dy1] != opponentColor && distance++ < 3) {
			if (board[x1][y1] == turn) curCount++;
			x1 += dx1;
			y1 += dy1;
		}

		distance = 0;
		while (isOnBoard(x2 + dx2, y2 + dy2) && board[x2 + dx2][y2 + dy2] != opponentColor && distance++ < 3) {
			if (board[x2][y2] == turn) curCount++;
			x2 += dx2;
			y2 += dy2;
		}

		if (curCount >= 3 && ++threeCount >= 2) return true;
	}
	return false;
}

Position Gomoku::_gomokuSolver(){
	auto start = std::chrono::high_resolution_clock::now();
	Position pos = _solver.solve(_board, _turn, _captureStone);
	auto end = std::chrono::high_resolution_clock::now();
	double duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	std::cout << YELLOW << "Execution Time: " << duration / 1e3 << " ms\n" << RESET;
	std::cout << "Solver: max searched depth " << _solver.getLastDepth()
		<< ", traversed nodes " << _solver.getLastNodes()
		<< ", state score " << _solver.getLastScore() << std::endl;

	solveTime.push_back(duration / 1e3);

	return pos;
}

std::vector<std::vector<uint8_t>> Gomoku::_updateBorad(int x, int y){
	if (_board[x][y] != 0) return _board;

	if (checkDoubleFreeThree(_board, _turn, x, y))
	{
		std::cout << "Forbidden move: double three is illegal" << std::endl;
		return _board;
	}

	_moveHistory.push_back(History{x, y, _turn});
	_board[x][y] = _turn;
	capture(_board, x, y, _turn, _captureStone);

	_winner = checkWinning(_board, _turn, _captureStone);
	if (_winner != 0) {
		_moveHistory.push_back(History{x, y, _turn});
		return _board;
	}

	_turn = 3 ^ _turn;
	_ui.getWindow().clear(sf::Color(255, 229, 204));
	_ui.draw(_board, _captureStone, _winnerRecord, _ai, _turn, _sugg ? _suggPos : Position{-1, -1}, _winner,
			(_sugg || _moveHistory.empty()) ? History{-1, -1, 0} : _moveHistory.back());
	if (_turn == 2 && _ai) {
		Position pos1 = _gomokuSolver();
		if (pos1.x < 0 || pos1.y < 0 || !isOnBoard(pos1.x, pos1.y)){
			std::cout << "AI found no move." << std::endl;
			_suggPos = Position({-1, -1});
			return _board;
		}
		x = pos1.x;
		y = pos1.y;
		_moveHistory.push_back(History{x, y, _turn});
		_board[x][y] = _turn;
		capture(_board, x, y, _turn, _captureStone);
		_winner = checkWinning(_board, _turn, _captureStone);
		_turn = 1;
	} else if (_sugg){
		_suggPos = _gomokuSolver();
	} else {
		_suggPos.x = -1;
		_suggPos.y = -1;
	}
	return _board;
}

int Gomoku::_winningHandler(){
	if (_winner == 0){ return false; }

	_ui.draw(_board, _captureStone, _winnerRecord, _ai, _turn, _sugg ? _suggPos : Position{-1, -1}, _winner,
			(_sugg || _moveHistory.empty()) ? History{-1, -1, 0} : _moveHistory.back());
	std::string winningMessage = _winner == 1? std::string("White win!") : std::string("Black win!");
	std::cout << winningMessage << std::endl;
	_winnerRecord[_winner - 1] += 1;
	sf::sleep(sf::seconds(3));
	double totalTime = std::reduce(solveTime.begin(), solveTime.end(), 0);
	std::cout << "Average time to find a move: " << totalTime / solveTime.size() << "ms" << std::endl;
	this->_reset();
	return true;
}

int Gomoku::run(){
	float gridSize = _ui.getGridSize();

	while (_ui.getWindow().isOpen()){
		while (const std::optional event = _ui.getWindow().pollEvent()){
			if (event->is<sf::Event::Closed>())
				_ui.getWindow().close();
			if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>())
			{
				if (mb->button == sf::Mouse::Button::Left){
					sf::Vector2i px = mb->position;

					if (px.x < gridSize || px.y < gridSize) continue;

					unsigned int x = std::round((px.x - gridSize * 1.5) / gridSize);
					unsigned int y = std::round((px.y - gridSize * 1.5) / gridSize);
					std::cout << "Player played at x: " << x << "    y: " << y << std::endl;
					if (x > SIZE - 1 || y > SIZE - 1) continue;
					_board = _updateBorad(x, y);
					/*printBoardCount();
					  _solver._rootPlayer = 3 ^ _turn;
					  _solver._board = _board;
					  _solver._evaluate();*/
					if (_winningHandler()) {}
				}
			}
			if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()){
				if (keyPressed->scancode == sf::Keyboard::Scan::R){
					std::cout << "Game is reset." << std::endl;
					this->_reset();
				} else if (keyPressed->scancode == sf::Keyboard::Scan::A){
					if (_ai) {_ai = false;}
					else {_ai = true;}
					std::cout << "AI is " << _ai << std::endl;
				} else if (keyPressed->scancode == sf::Keyboard::Scan::S){
					if (_sugg) {
						_sugg = false;
						std::cout << "AI suggestion is activated." << std::endl;
					} else {
						_sugg = true;
						std::cout << "AI suggestion is deactivated." << std::endl;
					}
				}
			}
		}
		_ui.getWindow().clear(sf::Color(255, 229, 204));
		_ui.draw(_board, _captureStone, _winnerRecord, _ai, _turn, _sugg ? _suggPos : Position{-1, -1}, _winner,
				(_sugg || _moveHistory.empty()) ? History{-1, -1, 0} : _moveHistory.back());
	}
	return 0;
}
