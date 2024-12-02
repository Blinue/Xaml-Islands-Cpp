#include "pch.h"
#include "AppSettings.h"
#include "Win32Helper.h"
#include <winrt/Windows.UI.ViewManagement.h>

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::ViewManagement;

namespace XamlIslandsCpp {

// 来自 https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/apply-windows-themes#know-when-dark-mode-is-enabled
static bool IsColorLight(const Color& clr) noexcept {
	return 5 * clr.G + 2 * clr.R + clr.B > 8 * 128;
}

AppSettings::AppSettings() {
	Color foregroundColor = UISettings().GetColorValue(UIColorType::Foreground);
	_theme = IsColorLight(foregroundColor) ? AppTheme::Dark : AppTheme::Light;

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
