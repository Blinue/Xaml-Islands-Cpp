#pragma once
#include "Minesweeper.g.h"
#include <bitset>
#include <vector>

namespace winrt::ClassLibrary::implementation {

struct Minesweeper : public MinesweeperT<Minesweeper> {
	void InitializeComponent();

	void RestartButton_Click(IInspectable const&, RoutedEventArgs const&);

	static constexpr uint32_t COLUMN_COUNT = 8;
	static constexpr uint32_t ROW_COUNT = 5;

private:
	void _UpdateCellButtonState(uint32_t idx) noexcept;

	void _CellButton_Click(IInspectable const&, RoutedEventArgs const&);

	void _CellButton_RightTapped(IInspectable const&, RightTappedRoutedEventArgs const&);

	std::vector<Button> _cellButtons;
	std::bitset<COLUMN_COUNT * ROW_COUNT> _flags;
};

}

namespace winrt::ClassLibrary::factory_implementation {

struct Minesweeper : MinesweeperT<Minesweeper, implementation::Minesweeper> {
};

}
