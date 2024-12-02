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
};

}
