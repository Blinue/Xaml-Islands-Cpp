#pragma once
#include "pch.h"
#include "RootPage.g.h"
#include "Settings.h"

namespace winrt::XamlIslandsCpp::App::implementation {

struct RootPage : RootPageT<RootPage> {
	RootPage();

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

	void ComboBox_DropDownOpened(IInspectable const&, IInspectable const&) const;

private:
	void _SetTheme(AppTheme theme);

	event<PropertyChangedEventHandler> _propertyChangedEvent;

	XamlIslandsCpp::App::Settings _settings{ nullptr };
};

}

namespace winrt::XamlIslandsCpp::App::factory_implementation {

struct RootPage : RootPageT<RootPage, implementation::RootPage> {
};

}
