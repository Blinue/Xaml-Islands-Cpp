#pragma once
#include <winrt/Windows.UI.Xaml.h>

namespace XamlIslandsCpp {

struct XamlHelper {
	static void RepositionXamlPopups(winrt::XamlRoot const& root, bool closeFlyoutPresenter);

	static void CloseComboBoxPopup(winrt::XamlRoot const& root);

	static void UpdateThemeOfXamlPopups(const winrt::XamlRoot& root, winrt::ElementTheme theme);

	static void UpdateThemeOfTooltips(const winrt::DependencyObject& root, winrt::ElementTheme theme);
};

}
