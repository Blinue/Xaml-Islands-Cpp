#include "pch.h"
#include "RootPage.h"
#if __has_include("RootPage.g.cpp")
#include "RootPage.g.cpp"
#endif
#include "Win32Helper.h"
#include "CommonSharedConstants.h"
#include "XamlHelper.h"

using namespace XamlIslandsCpp;

namespace winrt::XamlIslandsCpp::App::implementation {

RootPage::RootPage() {
	_settings = Application::Current().as<App>().Settings();
}

void RootPage::InitializeComponent() {
	RootPageT::InitializeComponent();

	_SetTheme(_settings.Theme(), _settings.Backdrop());
}

bool RootPage::IsCustomTitleBarEnabled() const noexcept {
	return _settings.IsCustomTitleBarEnabled();
}

void RootPage::IsCustomTitleBarEnabled(bool value) {
	_settings.IsCustomTitleBarEnabled(value);
	_propertyChangedEvent(*this, PropertyChangedEventArgs(L"IsCustomTitleBarEnabled"));
}

int RootPage::Theme() const noexcept {
	return (int)_settings.Theme();
}

void RootPage::Theme(int value) {
	if (value < 0) {
		return;
	}

	_settings.Theme((AppTheme)value);
	_SetTheme((AppTheme)value, _settings.Backdrop());
	_propertyChangedEvent(*this, PropertyChangedEventArgs(L"Theme"));
}

bool RootPage::IsMicaAvailable() noexcept {
	return Win32Helper::GetOSVersion().Is22H2OrNewer();
}

int RootPage::Backdrop() const noexcept {
	return (int)_settings.Backdrop();
}

void RootPage::Backdrop(int value) {
	if (value < 0) {
		return;
	}

	_settings.Backdrop((WindowBackdrop)value);
	_SetTheme(_settings.Theme(), (WindowBackdrop)value);
	_propertyChangedEvent(*this, PropertyChangedEventArgs(L"Backdrop"));
}

void RootPage::ComboBox_DropDownOpened(IInspectable const&, IInspectable const&) const {
	// 修复下拉框不适配主题的问题
	// https://github.com/microsoft/microsoft-ui-xaml/issues/6622
	XamlHelper::UpdateThemeOfXamlPopups(XamlRoot(), ActualTheme());
}

static Color Win32ColorToWinRTColor(COLORREF color) {
	return { 255, GetRValue(color), GetGValue(color), GetBValue(color) };
}

void RootPage::_SetTheme(AppTheme theme, WindowBackdrop backdrop) {
	const bool isDarkTheme = theme == AppTheme::Dark;

	ElementTheme newTheme = isDarkTheme ? ElementTheme::Dark : ElementTheme::Light;
	RequestedTheme(newTheme);

	if (Win32Helper::GetOSVersion().Is22H2OrNewer()) {
		if (backdrop != WindowBackdrop::SolidColor) {
			MUXC::BackdropMaterial::SetApplyToRootOrPageBackground(*this, true);
			return;
		}
			
		MUXC::BackdropMaterial::SetApplyToRootOrPageBackground(*this, false);
	}

	const Windows::UI::Color bkgColor = Win32ColorToWinRTColor(
		isDarkTheme ? CommonSharedConstants::DARK_TINT_COLOR : CommonSharedConstants::LIGHT_TINT_COLOR);
	Background(SolidColorBrush(bkgColor));
}

}
