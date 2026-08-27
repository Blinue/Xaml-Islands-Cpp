#include "pch.h"
#include "AppSettings.h"
#include "Win32Helper.h"

namespace XamlIslandsCpp {

AppSettings::AppSettings() {
	_backdrop = Win32Helper::GetOSVersion().Is22H2OrNewer() ?
		WindowBackdrop::Mica : WindowBackdrop::SolidColor;
}

void AppSettings::SetCustomTitleBarEnabled(bool value) {
	if (_isCustomTitleBarEnabled == value) {
		return;
	}

	_isCustomTitleBarEnabled = value;
	IsCustomTitleBarEnabledChanged.Invoke(value);
}

void AppSettings::SetTheme(AppTheme value) {
	if (_theme == value) {
		return;
	}

	_theme = value;
	ThemeChanged.Invoke(value);
}

void AppSettings::SetBackdrop(WindowBackdrop value) {
	if (_backdrop == value) {
		return;
	}

	_backdrop = value;
	BackdropChanged.Invoke(value);
}

}
