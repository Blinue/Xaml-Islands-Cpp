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

	_SetTheme(_settings.Theme());
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
	_SetTheme((AppTheme)value);
	_propertyChangedEvent(*this, PropertyChangedEventArgs(L"Theme"));
}

void RootPage::ComboBox_DropDownOpened(IInspectable const&, IInspectable const&) const {
	// 修复下拉框不适配主题的问题
	// https://github.com/microsoft/microsoft-ui-xaml/issues/6622
	XamlHelper::UpdateThemeOfXamlPopups(XamlRoot(), ActualTheme());
}

static Color Win32ColorToWinRTColor(COLORREF color) {
	return { 255, GetRValue(color), GetGValue(color), GetBValue(color) };
}

void RootPage::_SetTheme(AppTheme theme) {
	const bool isDarkTheme = theme == AppTheme::Dark;

	if (IsLoaded() && (ActualTheme() == ElementTheme::Dark) == isDarkTheme) {
		// 无需切换
		return;
	}

	if (Win32Helper::GetOSVersion().Is22H2OrNewer()) {
		// Win11 22H2+ 使用系统的 Mica 背景
		MUXC::BackdropMaterial::SetApplyToRootOrPageBackground(*this, true);
	} else {
		const Windows::UI::Color bkgColor = Win32ColorToWinRTColor(
			isDarkTheme ? CommonSharedConstants::DARK_TINT_COLOR : CommonSharedConstants::LIGHT_TINT_COLOR);
		Background(SolidColorBrush(bkgColor));
	}

	ElementTheme newTheme = isDarkTheme ? ElementTheme::Dark : ElementTheme::Light;
	RequestedTheme(newTheme);
}

}
