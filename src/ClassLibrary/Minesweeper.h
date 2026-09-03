#pragma once
#include "Minesweeper.g.h"
#include <vector>

namespace winrt::ClassLibrary::implementation {

struct Minesweeper : public MinesweeperT<Minesweeper> {
	void InitializeComponent();

	event_token PropertyChanged(PropertyChangedEventHandler const& handler) {
		return _propertyChangedEvent.add(handler);
	}

	void PropertyChanged(event_token const& token) noexcept {
		_propertyChangedEvent.remove(token);
	}

	hstring MinecountText() const noexcept;

	void RestartButton_Click(IInspectable const&, RoutedEventArgs const&);

	static constexpr uint32_t COLUMN_COUNT = 9;
	static constexpr uint32_t ROW_COUNT = 5;

private:
	void _UpdateCellButtonState(uint32_t idx) noexcept;

	void _CellButton_Click(IInspectable const&, RoutedEventArgs const&);

	void _CellButton_RightTapped(IInspectable const&, RightTappedRoutedEventArgs const&);

	event<PropertyChangedEventHandler> _propertyChangedEvent;
	ResourceLoader _resourceLoader{ nullptr };
	std::vector<Button> _cellButtons;
};

}

namespace winrt::ClassLibrary::factory_implementation {

struct Minesweeper : MinesweeperT<Minesweeper, implementation::Minesweeper> {
};

}
