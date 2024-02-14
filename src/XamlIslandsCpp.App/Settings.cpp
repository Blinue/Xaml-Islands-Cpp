#include "pch.h"
#include "Settings.h"
#if __has_include("Settings.g.cpp")
#include "Settings.g.cpp"
#endif
#include <winrt/Windows.UI.ViewManagement.h>

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
}

}
