#pragma once
#include "Event.h"

namespace XamlIslandsCpp {

enum class AppTheme {
	System,
	Light,
	Dark
};

enum class WindowBackdrop {
	SolidColor,
	Acrylic,
	Mica,
	MicaAlt
};

class AppSettings {
public:
	AppSettings(const AppSettings&) = delete;
	AppSettings(AppSettings&&) = delete;

	static AppSettings& Get() {
		static AppSettings instance;
		return instance;
	}

	bool IsCustomTitleBarEnabled() const noexcept {
		return _isCustomTitleBarEnabled;
	}

	void SetCustomTitleBarEnabled(bool value);

	AppTheme GetTheme() const noexcept {
		return _theme;
	}
	void SetTheme(AppTheme value);

	WindowBackdrop GetBackdrop() const noexcept {
		return _backdrop;
	}

	void SetBackdrop(WindowBackdrop value);

	Event<bool> IsCustomTitleBarEnabledChanged;
	Event<AppTheme> ThemeChanged;
	Event<WindowBackdrop> BackdropChanged;
	
private:
	AppSettings();

	bool _isCustomTitleBarEnabled = true;
	AppTheme _theme = AppTheme::System;
	WindowBackdrop _backdrop = WindowBackdrop::SolidColor;
};

}
