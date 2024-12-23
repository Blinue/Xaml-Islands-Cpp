#include "pch.h"
#include "AppSettings.h"
#include "Win32Helper.h"
#include <winrt/Windows.UI.ViewManagement.h>

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::ViewManagement;

namespace XamlIslandsCpp {

AppSettings::AppSettings() {
	_backdrop = Win32Helper::GetOSVersion().Is22H2OrNewer() ?
		WindowBackdrop::Mica : WindowBackdrop::SolidColor;
}

void AppSettings::IsCustomTitleBarEnabled(bool value) {
	if (_isCustomTitleBarEnabled == value) {
		return;
	}

	_isCustomTitleBarEnabled = value;
	IsCustomTitleBarEnabledChanged.Invoke(value);
}

void AppSettings::Theme(AppTheme value) {
	if (_theme == value) {
		return;
	}

	_theme = value;
	ThemeChanged.Invoke(value);
}

void AppSettings::Backdrop(WindowBackdrop value) {
	if (_backdrop == value) {
		return;
	}

	_backdrop = value;
	BackdropChanged.Invoke(value);
}

}
