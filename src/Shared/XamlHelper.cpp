#include "pch.h"
#include "XamlHelper.h"
#include <winrt/Windows.UI.Xaml.Media.h>

namespace winrt {
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Media;
}

namespace XamlIslandsCpp {

void XamlHelper::RepositionXamlPopups(winrt::XamlRoot const& root, bool closeFlyoutPresenter) {
	if (!root) {
		return;
	}

	for (const auto& popup : winrt::VisualTreeHelper::GetOpenPopupsForXamlRoot(root)) {
		if (closeFlyoutPresenter) {
			auto className = winrt::get_class_name(popup.Child());
			if (className == winrt::name_of<winrt::Controls::FlyoutPresenter>() ||
				className == winrt::name_of<winrt::Controls::MenuFlyoutPresenter>()
			) {
				popup.IsOpen(false);
				continue;
			}
		}

		// 取自 https://github.com/CommunityToolkit/Microsoft.Toolkit.Win32/blob/229fa3cd245ff002906b2a594196b88aded25774/Microsoft.Toolkit.Forms.UI.XamlHost/WindowsXamlHostBase.cs#L180

		// Toggle the CompositeMode property, which will force all windowed Popups
		// to reposition themselves relative to the new position of the host window.
		auto compositeMode = popup.CompositeMode();

		// Set CompositeMode to some value it currently isn't set to.
		if (compositeMode == winrt::ElementCompositeMode::SourceOver) {
			popup.CompositeMode(winrt::ElementCompositeMode::MinBlend);
		} else {
			popup.CompositeMode(winrt::ElementCompositeMode::SourceOver);
		}

		// Restore CompositeMode to whatever it was originally set to.
		popup.CompositeMode(compositeMode);
	}
}

static bool IsComboBoxPopup(const winrt::Popup& popup) {
	winrt::UIElement child = popup.Child();
	if (!child.try_as<winrt::Canvas>()) {
		return false;
	}

	// 查找 XAML 树中是否存在 ComboBoxItem
	std::vector<winrt::DependencyObject> elems{ std::move(child) };
	do {
		std::vector<winrt::DependencyObject> temp;

		for (const winrt::DependencyObject& elem : elems) {
			const int count = winrt::VisualTreeHelper::GetChildrenCount(elem);
			for (int i = 0; i < count; ++i) {
				winrt::DependencyObject current = winrt::VisualTreeHelper::GetChild(elem, i);

				if (current.try_as<winrt::ComboBoxItem>()) {
					return true;
				}

				temp.emplace_back(std::move(current));
			}
		}

		elems = std::move(temp);
	} while (!elems.empty());

	return false;
}

void XamlHelper::CloseComboBoxPopup(winrt::XamlRoot const& root) {
	for (const winrt::Popup& popup : winrt::VisualTreeHelper::GetOpenPopupsForXamlRoot(root)) {
		if (IsComboBoxPopup(popup)) {
			popup.IsOpen(false);
			return;
		}
	}
}

}
