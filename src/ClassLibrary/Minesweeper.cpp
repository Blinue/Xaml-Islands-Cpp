#include "pch.h"
#include "Minesweeper.h"
#if __has_include("Minesweeper.g.cpp")
#include "Minesweeper.g.cpp"
#endif
#include <format>
#include <random>

namespace winrt::ClassLibrary::implementation {

static constexpr const wchar_t* RESOURCE_MAP_ID = L"ClassLibrary/Resources";

static constexpr uint32_t ROW_COUNT = Minesweeper::ROW_COUNT;
static constexpr uint32_t COLUMN_COUNT = Minesweeper::COLUMN_COUNT;
static constexpr uint32_t CELL_COUNT = COLUMN_COUNT * ROW_COUNT;
static constexpr uint32_t MINE_COUNT = 8;
static_assert(MINE_COUNT + 1 < CELL_COUNT);

struct CellData {
	bool IsMine() const noexcept {
		return number < 0;
	}

	bool IsBlank() const noexcept {
		return number == 0;
	}

	// -1 表示雷
	int number = 0;
	bool isOpened = false;
	bool isFlaged = false;
};

class GameState {
public:
	GameState(const GameState&) = delete;
	GameState(GameState&&) = delete;

	static GameState& Get() {
		static GameState instance;
		return instance;
	}

	bool IsGameOver() const noexcept {
		return _isGameOver;
	}

	bool IsGameWon() const noexcept {
		return _isGameWon;
	}

	int GetRemainingMineCount() const noexcept {
		return _remainingMineCount;
	}

	// 返回状态改变的单元编号
	std::vector<uint32_t> OpenCell(uint32_t idx) noexcept {
		assert(!_isGameOver && !_isGameWon);

		// 第一次点击时生成地雷
		if (!_isStarted) {
			_InitializeBoard(idx);
			_isStarted = true;
		}

		CellData& curCellData = _cells[idx];
		assert(!curCellData.isOpened);
		curCellData.isOpened = true;

		if (curCellData.isFlaged) {
			++_remainingMineCount;
		}

		std::vector<uint32_t> outdatedCells;
		outdatedCells.push_back(idx);

		if (curCellData.IsMine()) {
			// 游戏结束，显示所有地雷
			_isGameOver = true;

			for (uint32_t i = 0; i < CELL_COUNT; ++i) {
				if (i != idx && _cells[i].IsMine()) {
					_cells[i].isOpened = true;
					outdatedCells.push_back(i);
				}
			}
		} else {
			if (curCellData.IsBlank()) {
				// 点击空白时显示相邻的所有空白
				std::vector<uint32_t> stack(1, idx);
				do {
					uint32_t curIdx = stack.back();
					stack.pop_back();

					if (curIdx != idx) {
						if (std::exchange(_cells[curIdx].isOpened, true)) {
							continue;
						}

						if (curCellData.isFlaged) {
							++_remainingMineCount;
						}

						outdatedCells.push_back(curIdx);

						if (!_cells[curIdx].IsBlank()) {
							continue;
						}
					}

					uint32_t col = curIdx % COLUMN_COUNT;
					uint32_t row = curIdx / COLUMN_COUNT;

					// 检查周围 8 格，如果只检查 4 格那么角落的数字不会显示
					bool hasTop = row > 0;
					bool hasBottom = row < ROW_COUNT - 1;

					if (col > 0) {
						if (hasTop) {
							stack.push_back(curIdx - 1 - COLUMN_COUNT);
						}

						stack.push_back(curIdx - 1);

						if (hasBottom) {
							stack.push_back(curIdx - 1 + COLUMN_COUNT);
						}
					}
					
					if (hasTop) {
						stack.push_back(curIdx - COLUMN_COUNT);
					}

					if (hasBottom) {
						stack.push_back(curIdx + COLUMN_COUNT);
					}

					if (col < COLUMN_COUNT - 1) {
						if (hasTop) {
							stack.push_back(curIdx + 1 - COLUMN_COUNT);
						}

						stack.push_back(curIdx + 1);

						if (hasBottom) {
							stack.push_back(curIdx + 1 + COLUMN_COUNT);
						}
					}
				} while (!stack.empty());
			}

			// 如果通关则显示所有地雷
			_isGameWon = true;
			for (uint32_t i = 0; i < CELL_COUNT; ++i) {
				if (!_cells[i].isOpened && !_cells[i].IsMine()) {
					_isGameWon = false;
					break;
				}
			}
			if (_isGameWon) {
				_remainingMineCount = 0;

				for (uint32_t i = 0; i < CELL_COUNT; ++i) {
					if (_cells[i].IsMine()) {
						_cells[i].isOpened = true;
						outdatedCells.push_back(i);
					}
				}
			}
		}

		return outdatedCells;
	}

	void FlagCell(uint32_t idx) noexcept {
		assert(!_isGameOver && !_isGameWon);

		CellData& curCellData = _cells[idx];
		curCellData.isFlaged = !curCellData.isFlaged;

		if (curCellData.isFlaged) {
			--_remainingMineCount;
		} else {
			++_remainingMineCount;
		}
	}

	const CellData& GetCellData(uint32_t idx) const noexcept {
		return _cells[idx];
	}

	void Restart() noexcept {
		_remainingMineCount = MINE_COUNT;
		_isStarted = false;
		_isGameOver = false;
		_isGameWon = false;
		std::fill(_cells.begin(), _cells.end(), CellData{});
	}

private:
	GameState() = default;

	void _InitializeBoard(uint32_t safeCell) noexcept {
		// 随机生成地雷，不会在 safeCell 处生成
		std::vector<uint32_t> list(CELL_COUNT - 1);
		uint32_t listIdx = 0;
		for (uint32_t i = 0; i < CELL_COUNT; ++i) {
			if (i != safeCell) {
				list[listIdx++] = i;
			}
		}

		static int _ = [this] {
			std::random_device randomDevice;
			_randomEngine.seed(randomDevice());
			return 0;
		}();

		std::shuffle(list.begin(), list.end(), _randomEngine);

		for (uint32_t i = 0; i < MINE_COUNT; ++i) {
			_cells[list[i]].number = -1;
		}

		// 计算每个网格的数字
		for (uint32_t i = 0; i < CELL_COUNT; ++i) {
			int& curNumber = _cells[i].number;

			if (curNumber == -1) {
				continue;
			}

			uint32_t col = i % COLUMN_COUNT;
			uint32_t row = i / COLUMN_COUNT;

			bool hasTop = row > 0;
			bool hasBottom = row < ROW_COUNT - 1;

			if (col > 0) {
				if (hasTop && _cells[size_t(i - 1 - COLUMN_COUNT)].IsMine()) {
					++curNumber;
				}

				if (_cells[size_t(i - 1)].IsMine()) {
					++curNumber;
				}

				if (hasBottom && _cells[size_t(i - 1 + COLUMN_COUNT)].IsMine()) {
					++curNumber;
				}
			}
			
			if (hasTop && _cells[size_t(i - COLUMN_COUNT)].IsMine()) {
				++curNumber;
			}

			if (hasBottom && _cells[size_t(i + COLUMN_COUNT)].IsMine()) {
				++curNumber;
			}

			if (col < COLUMN_COUNT - 1) {
				if (hasTop && _cells[size_t(i + 1 - COLUMN_COUNT)].IsMine()) {
					++curNumber;
				}

				if (_cells[size_t(i + 1)].IsMine()) {
					++curNumber;
				}

				if (hasBottom && _cells[size_t(i + 1 + COLUMN_COUNT)].IsMine()) {
					++curNumber;
				}
			}
		}
	}

	std::array<CellData, CELL_COUNT> _cells;
	std::default_random_engine _randomEngine;
	// 可能是负数
	int _remainingMineCount = MINE_COUNT;
	bool _isStarted = false;
	bool _isGameOver = false;
	bool _isGameWon = false;
};

void Minesweeper::InitializeComponent() {
	MinesweeperT::InitializeComponent();

	// 演示读取字符串资源
	_resourceLoader = ResourceLoader::GetForViewIndependentUse(RESOURCE_MAP_ID);

	const Grid& boardGrid = BoardGrid();
	const auto& colDefinitions = boardGrid.ColumnDefinitions();
	const auto& rowDefinitions = boardGrid.RowDefinitions();

	// 初始化网格
	for (uint32_t i = 0; i < COLUMN_COUNT; ++i) {
		ColumnDefinition colDefinition;
		colDefinition.Width({ 1.0, GridUnitType::Star });
		colDefinitions.Append(colDefinition);
	}

	for (uint32_t i = 0; i < ROW_COUNT; ++i) {
		RowDefinition rowDefinition;
		rowDefinition.Height({ 1.0, GridUnitType::Star });
		rowDefinitions.Append(rowDefinition);
	}
	
	_cellButtons.resize(CELL_COUNT);

	const auto& buttonCollection = boardGrid.Children();
	for (int i = 0; i < CELL_COUNT; ++i) {
		const Button& curCellButton = _cellButtons[i];
		buttonCollection.Append(curCellButton);

		Grid::SetColumn(curCellButton, i % COLUMN_COUNT);
		Grid::SetRow(curCellButton, i / COLUMN_COUNT);
		
		curCellButton.Click({ this, &Minesweeper::_CellButton_Click });
		curCellButton.RightTapped({ this, &Minesweeper::_CellButton_RightTapped });

		_UpdateCellButtonState(i);
	}
}

hstring Minesweeper::MinecountText() const noexcept {
	hstring templateStr = _resourceLoader.GetString(L"MinecountText");
	int minecount = GameState::Get().GetRemainingMineCount();
	return hstring(std::vformat(templateStr, std::make_wformat_args(minecount)));
}

void Minesweeper::RestartButton_Click(IInspectable const&, RoutedEventArgs const&) {
	GameState::Get().Restart();

	for (int i = 0; i < CELL_COUNT; ++i) {
		_UpdateCellButtonState(i);
	}

	_propertyChangedEvent(*this, PropertyChangedEventArgs(L"MinecountText"));
}

void Minesweeper::_UpdateCellButtonState(uint32_t idx) noexcept {
	const Button& button = _cellButtons[idx];
	const CellData& cellData = GameState::Get().GetCellData(idx);

	button.IsEnabled(!cellData.isOpened);

	if (cellData.isOpened) {
		if (cellData.IsMine()) {
			button.Content(box_value(GameState::Get().IsGameWon() ? L"🎉" : L"💥"));
		} else if (cellData.IsBlank()) {
			button.Content(nullptr);
		} else{
			button.Content(box_value(to_hstring(cellData.number)));
		}
	} else {
		button.Content(cellData.isFlaged ? box_value(L"🚩") : nullptr);
	}
}

void Minesweeper::_CellButton_Click(IInspectable const& sender, RoutedEventArgs const&) {
	// 游戏结束后不再响应点击
	if (GameState::Get().IsGameOver()) {
		return;
	}

	Button btn = sender.try_as<Button>();
	int col = Grid::GetColumn(btn);
	int row = Grid::GetRow(btn);

	for (uint32_t idx : GameState::Get().OpenCell(row * COLUMN_COUNT + col)) {
		_UpdateCellButtonState(idx);
	}

	_propertyChangedEvent(*this, PropertyChangedEventArgs(L"MinecountText"));
}

void Minesweeper::_CellButton_RightTapped(IInspectable const& sender, RightTappedRoutedEventArgs const&) {
	// 游戏结束后不再响应点击
	if (GameState::Get().IsGameOver()) {
		return;
	}

	Button btn = sender.try_as<Button>();
	int col = Grid::GetColumn(btn);
	int row = Grid::GetRow(btn);
	uint32_t idx = row * COLUMN_COUNT + col;

	GameState::Get().FlagCell(idx);
	_UpdateCellButtonState(idx);

	_propertyChangedEvent(*this, PropertyChangedEventArgs(L"MinecountText"));
}

}
