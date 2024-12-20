#pragma once
#include "pch.h"
#include "RootPage.g.h"
#include "TitleBarControl.h"

namespace winrt::XamlIslandsCpp::implementation {

struct RootPage : RootPageT<RootPage> {
	void InitializeComponent();

	event_token PropertyChanged(PropertyChangedEventHandler const& handler) {
		return _propertyChangedEvent.add(handler);
	}

	void PropertyChanged(event_token const& token) noexcept {
		_propertyChangedEvent.remove(token);
	}

	bool IsCustomTitleBarEnabled() const noexcept;
	void IsCustomTitleBarEnabled(bool value);

	int Theme() const noexcept;
	void Theme(int value);

	static bool IsMicaAvailable() noexcept;

	int Backdrop() const noexcept;
	void Backdrop(int value);

	TitleBarControl& TitleBar() noexcept {
		return *get_self<TitleBarControl>(RootPageT::TitleBar());
	}

	void ComboBox_DropDownOpened(IInspectable const&, IInspectable const&) const;

private:
	void _UpdateTheme();

	event<PropertyChangedEventHandler> _propertyChangedEvent;
};

}

namespace winrt::XamlIslandsCpp::factory_implementation {

struct RootPage : RootPageT<RootPage, implementation::RootPage> {
};

}
