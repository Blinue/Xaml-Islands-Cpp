#pragma once
#include <winrt/Windows.UI.Xaml.h>

namespace XamlIslandsCpp {

struct XamlHelper {
	static void RepositionXamlPopups(winrt::Windows::UI::Xaml::XamlRoot const& root, bool closeFlyoutPresenter);
	static void CloseComboBoxPopup(winrt::Windows::UI::Xaml::XamlRoot const& root);
};

}
