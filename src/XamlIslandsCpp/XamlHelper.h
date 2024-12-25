#pragma once
#include <winrt/Windows.UI.Xaml.h>

namespace XamlIslandsCpp {

struct XamlHelper {
	static void RepositionXamlPopups(winrt::XamlRoot const& root, bool closeFlyoutPresenter);

	static void CloseComboBoxPopup(winrt::XamlRoot const& root);

	static void UpdateThemeOfXamlPopups(const winrt::XamlRoot& root, winrt::ElementTheme theme);

	static void UpdateThemeOfTooltips(const winrt::DependencyObject& root, winrt::ElementTheme theme);

	// 遍历 XAML 树跳过所有动画，如果 UI 复杂会相当慢（超过 100ms）
	static void SkipAnimations(const winrt::DependencyObject& root);
};

}
