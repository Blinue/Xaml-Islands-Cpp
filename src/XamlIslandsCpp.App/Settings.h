#pragma once
#include "Settings.g.h"

namespace winrt::XamlIslandsCpp::App::implementation {

struct Settings : SettingsT<Settings> {
    Settings();

	bool IsCustomTitleBarEnabled() const noexcept {
		return _isCustomTitleBarEnabled;
	}

	void IsCustomTitleBarEnabled(bool value);

	event_token IsCustomTitleBarEnabledChanged(EventHandler<bool> const& handler) {
		return _isCustomTitleBarEnabledChangedEvent.add(handler);
	}

	void IsCustomTitleBarEnabledChanged(event_token const& token) {
		_isCustomTitleBarEnabledChangedEvent.remove(token);
	}

	AppTheme Theme() const noexcept {
		return _theme;
	}

	void Theme(AppTheme value);

	event_token ThemeChanged(EventHandler<AppTheme> const& handler) {
		return _appThemeChangedEvent.add(handler);
	}

	void ThemeChanged(event_token const& token) {
		_appThemeChangedEvent.remove(token);
	}

private:
	event<EventHandler<AppTheme>> _appThemeChangedEvent;
	event<EventHandler<bool>> _isCustomTitleBarEnabledChangedEvent;

	AppTheme _theme = AppTheme::Light;
	bool _isCustomTitleBarEnabled = true;
};

}

namespace winrt::XamlIslandsCpp::App::factory_implementation {

struct Settings : SettingsT<Settings, implementation::Settings> {
};

}
