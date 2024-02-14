#pragma once
#include "Settings.g.h"

namespace winrt::XamlIslandsCpp::App::implementation {

struct Settings : SettingsT<Settings> {
    Settings();

	AppTheme Theme() const noexcept {
		return _theme;
	}
	void Theme(AppTheme value) {
		if (_theme == value) {
			return;
		}

		_theme = value;
		_appThemeChangedEvent(*this, value);
	}

	event_token ThemeChanged(EventHandler<AppTheme> const& handler) {
		return _appThemeChangedEvent.add(handler);
	}

	void ThemeChanged(event_token const& token) {
		_appThemeChangedEvent.remove(token);
	}

private:
	event<EventHandler<AppTheme>> _appThemeChangedEvent;

	AppTheme _theme = AppTheme::Light;
};

}

namespace winrt::XamlIslandsCpp::App::factory_implementation {

struct Settings : SettingsT<Settings, implementation::Settings> {
};

}
