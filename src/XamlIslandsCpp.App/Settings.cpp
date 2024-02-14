#include "pch.h"
#include "Settings.h"
#if __has_include("Settings.g.cpp")
#include "Settings.g.cpp"
#endif
#include <winrt/Windows.UI.ViewManagement.h>
#include "Win32Helper.h"

using namespace ::XamlIslandsCpp;
using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::ViewManagement;

namespace winrt::XamlIslandsCpp::App::implementation {

// 来自 https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/apply-windows-themes#know-when-dark-mode-is-enabled
static bool IsColorLight(const Color& clr) noexcept {
	return 5 * clr.G + 2 * clr.R + clr.B > 8 * 128;
}

Settings::Settings() {
	Color foregroundColor = UISettings().GetColorValue(UIColorType::Foreground);
	_theme = IsColorLight(foregroundColor) ? AppTheme::Dark : AppTheme::Light;

	_backdrop = Win32Helper::GetOSVersion().Is22H2OrNewer() ?
		WindowBackdrop::Mica : WindowBackdrop::SolidColor;
}

void Settings::IsCustomTitleBarEnabled(bool value) {
	if (_isCustomTitleBarEnabled == value) {
		return;
	}

	_isCustomTitleBarEnabled = value;
	_isCustomTitleBarEnabledChangedEvent(*this, value);
}

void Settings::Theme(AppTheme value) {
	if (_theme == value) {
		return;
	}

	_theme = value;
	_appThemeChangedEvent(*this, value);
}

void Settings::Backdrop(WindowBackdrop value) {
	if (_backdrop == value) {
		return;
	}

	_backdrop = value;
	_backdropChangedEvent(*this, value);
}

}
