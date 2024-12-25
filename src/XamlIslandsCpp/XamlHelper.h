#pragma once
#include <winrt/Windows.UI.Xaml.h>

namespace XamlIslandsCpp {

struct XamlHelper {
	static void RepositionXamlPopups(winrt::Windows::UI::Xaml::XamlRoot const& root, bool closeFlyoutPresenter);

	static void CloseComboBoxPopup(winrt::Windows::UI::Xaml::XamlRoot const& root);

	static void UpdateThemeOfXamlPopups(
		const winrt::Windows::UI::Xaml::XamlRoot& root,
		winrt::Windows::UI::Xaml::ElementTheme theme
	);

	static void UpdateThemeOfTooltips(
		const winrt::Windows::UI::Xaml::DependencyObject& root,
		winrt::Windows::UI::Xaml::ElementTheme theme
	);

	// 遍历 XAML 树跳过所有动画，如果 UI 复杂会相当慢（超过 100ms）
	static void SkipAnimations(const winrt::Windows::UI::Xaml::DependencyObject& root);
};

}
