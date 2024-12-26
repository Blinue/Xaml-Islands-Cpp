#include "pch.h"
#include "RootPage.h"
#if __has_include("RootPage.g.cpp")
#include "RootPage.g.cpp"
#endif
#include "Win32Helper.h"
#include "CommonSharedConstants.h"
#include "XamlHelper.h"
#include "AppSettings.h"
#include "App.h"
#include "MainWindow.h"
#include "TitleBarControl.h"

using namespace ::XamlIslandsCpp;
using namespace winrt;
using namespace Windows::UI::Xaml::Media::Animation;

namespace winrt::XamlIslandsCpp::implementation {

static constexpr const wchar_t* RESOURCE_MAP_ID = L"XamlIslandsCpp/Resources";

void RootPage::InitializeComponent() {
	// 设置 Language 属性可以帮助 XAML 选择合适的字体，比如繁体中文使用 Microsoft JhengHei UI，日语使用 Yu Gothic UI
	// Language(L"en-us");

	RootPageT::InitializeComponent();

	_appThemeChangedRevoker = App::Get().ThemeChanged(auto_revoke, [this](bool) { _UpdateTheme(); });
	_UpdateTheme();

	ResourceLoader resourceLoader = ResourceLoader::GetForCurrentView(RESOURCE_MAP_ID);
	StringResTextBlock().Text(resourceLoader.GetString(L"Hello"));
}

static void SkipToggleSwitchAnimations(const DependencyObject& elem) {
	FrameworkElement rootGrid = VisualTreeHelper::GetChild(elem, 0).try_as<FrameworkElement>();

	for (VisualStateGroup group : VisualStateManager::GetVisualStateGroups(rootGrid)) {
		for (VisualState state : group.States()) {
			if (Storyboard storyboard = state.Storyboard()) {
				storyboard.SkipToFill();
			}
		}
	}
}

void RootPage::RootPage_Loaded(IInspectable const&, IInspectable const&) {
	// 启动时跳过 ToggleSwitch 的动画
	std::vector<DependencyObject> elems{ *this };
	do {
		std::vector<DependencyObject> temp;

		for (const DependencyObject& elem : elems) {
			const int count = VisualTreeHelper::GetChildrenCount(elem);
			for (int i = 0; i < count; ++i) {
				DependencyObject current = VisualTreeHelper::GetChild(elem, i);

				if (get_class_name(current) == name_of<ToggleSwitch>()) {
					SkipToggleSwitchAnimations(current);
				} else {
					temp.emplace_back(std::move(current));
				}
			}
		}

		elems = std::move(temp);
	} while (!elems.empty());
}

bool RootPage::IsCustomTitleBarEnabled() const noexcept {
	return AppSettings::Get().IsCustomTitleBarEnabled();
}

void RootPage::IsCustomTitleBarEnabled(bool value) {
	AppSettings::Get().IsCustomTitleBarEnabled(value);
	_propertyChangedEvent(*this, PropertyChangedEventArgs(L"IsCustomTitleBarEnabled"));
}

int RootPage::Theme() const noexcept {
	return (int)AppSettings::Get().Theme();
}

void RootPage::Theme(int value) {
	if (value < 0) {
		return;
	}

	AppSettings::Get().Theme((AppTheme)value);
	_propertyChangedEvent(*this, PropertyChangedEventArgs(L"Theme"));
}

bool RootPage::IsMicaAvailable() noexcept {
	return Win32Helper::GetOSVersion().Is22H2OrNewer();
}

int RootPage::Backdrop() const noexcept {
	return (int)AppSettings::Get().Backdrop();
}

void RootPage::Backdrop(int value) {
	if (value < 0) {
		return;
	}

	AppSettings::Get().Backdrop((WindowBackdrop)value);
	_UpdateTheme();

	_propertyChangedEvent(*this, PropertyChangedEventArgs(L"Backdrop"));
}

TitleBarControl& RootPage::TitleBar() noexcept {
	return *get_self<TitleBarControl>(RootPageT::TitleBar());
}

void RootPage::ComboBox_DropDownOpened(IInspectable const&, IInspectable const&) const {
	// 修复下拉框不适配主题的问题
	// https://github.com/microsoft/microsoft-ui-xaml/issues/6622
	XamlHelper::UpdateThemeOfXamlPopups(XamlRoot(), ActualTheme());
}

static Color Win32ColorToWinRTColor(COLORREF color) noexcept {
	return { 255, GetRValue(color), GetGValue(color), GetBValue(color) };
}

void RootPage::_UpdateTheme() {
	const bool isLightTheme = App::Get().IsLightTheme();
	const WindowBackdrop backdrop = AppSettings::Get().Backdrop();

	ElementTheme newTheme = isLightTheme ? ElementTheme::Light : ElementTheme::Dark;
	RequestedTheme(newTheme);

	if (Win32Helper::GetOSVersion().Is22H2OrNewer()) {
		if (backdrop != WindowBackdrop::SolidColor) {
			MUXC::BackdropMaterial::SetApplyToRootOrPageBackground(*this, true);
			return;
		}
			
		MUXC::BackdropMaterial::SetApplyToRootOrPageBackground(*this, false);
	}
	
	const Windows::UI::Color bkgColor = Win32ColorToWinRTColor(
		isLightTheme ? CommonSharedConstants::LIGHT_TINT_COLOR : CommonSharedConstants::DARK_TINT_COLOR);

	if (backdrop == WindowBackdrop::SolidColor) {
		Background(SolidColorBrush(bkgColor));
	} else {
		// 22H2 之前的系统使用背景刷子实现 Acrylic
		assert(backdrop == WindowBackdrop::Acrylic);
		AcrylicBrush brush;
		brush.BackgroundSource(AcrylicBackgroundSource::HostBackdrop);
		// 来自 https://github.com/microsoft/microsoft-ui-xaml/blob/75f7666f5907aad29de1cb2e49405cc06d433fba/dev/Materials/Acrylic/AcrylicBrush_19h1_themeresources.xaml#L12
		brush.TintColor(isLightTheme ? Color{ 255,252,252,252 } : Color{ 255,44,44,44 });
		brush.TintOpacity(isLightTheme ? 0.0 : 0.15);
		brush.FallbackColor(bkgColor);
		Background(brush);

		// 切换前台窗口以刷新背景
		SetForegroundWindow(GetDesktopWindow());
		SetForegroundWindow(App::Get().MainWindow().Handle());
	}
}

}
