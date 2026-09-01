#pragma once
#include <winrt/Windows.UI.Xaml.h>

namespace XamlIslandsCpp {

struct XamlHelper {
	// 使 XAML Islands 背景透明
	static void SetWindowBackgroundTransparency(const winrt::Window& window, bool transparent) noexcept;

	static void RepositionXamlPopups(winrt::XamlRoot const& root, bool closeFlyoutPresenter);

	static void CloseComboBoxPopup(winrt::XamlRoot const& root);

	static void UpdateThemeOfXamlPopups(const winrt::XamlRoot& root, winrt::ElementTheme theme);

	static void UpdateThemeOfTooltips(const winrt::DependencyObject& root, winrt::ElementTheme theme);
};

}
