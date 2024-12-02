#pragma once
#include "WinRTHelper.h"

namespace XamlIslandsCpp {

enum class AppTheme {
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

	void IsCustomTitleBarEnabled(bool value);

	AppTheme Theme() const noexcept {
		return _theme;
	}
	void Theme(AppTheme value);

	WindowBackdrop Backdrop() const noexcept {
		return _backdrop;
	}

	void Backdrop(WindowBackdrop value);

	WinRTHelper::Event<winrt::delegate<bool>> IsCustomTitleBarEnabledChanged;
	WinRTHelper::Event<winrt::delegate<AppTheme>> ThemeChanged;
	WinRTHelper::Event<winrt::delegate<WindowBackdrop>> BackdropChanged;
	
private:
	AppSettings();

	bool _isCustomTitleBarEnabled = true;
	AppTheme _theme = AppTheme::Light;
	WindowBackdrop _backdrop = WindowBackdrop::SolidColor;
};

}
