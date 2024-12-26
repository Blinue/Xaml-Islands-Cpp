#include "pch.h"
#include "XamlHelper.h"
#include "Win32Helper.h"
#include <winrt/Windows.UI.Xaml.Media.h>

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Media;

namespace XamlIslandsCpp {

void XamlHelper::RepositionXamlPopups(XamlRoot const& root, bool closeFlyoutPresenter) {
	if (!root) {
		return;
	}

	for (const auto& popup : VisualTreeHelper::GetOpenPopupsForXamlRoot(root)) {
		if (closeFlyoutPresenter) {
			auto className = get_class_name(popup.Child());
			if (className == name_of<FlyoutPresenter>() ||
				className == name_of<MenuFlyoutPresenter>()
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
		if (compositeMode == ElementCompositeMode::SourceOver) {
			popup.CompositeMode(ElementCompositeMode::MinBlend);
		} else {
			popup.CompositeMode(ElementCompositeMode::SourceOver);
		}

		// Restore CompositeMode to whatever it was originally set to.
		popup.CompositeMode(compositeMode);
	}
}

static bool IsComboBoxPopup(const Popup& popup) {
	UIElement child = popup.Child();
	if (!child.try_as<Canvas>()) {
		return false;
	}

	// 查找 XAML 树中是否存在 ComboBoxItem
	std::vector<DependencyObject> elems{ std::move(child) };
	do {
		std::vector<DependencyObject> temp;

		for (const DependencyObject& elem : elems) {
			const int count = VisualTreeHelper::GetChildrenCount(elem);
			for (int i = 0; i < count; ++i) {
				DependencyObject current = VisualTreeHelper::GetChild(elem, i);

				if (current.try_as<ComboBoxItem>()) {
					return true;
				}

				temp.emplace_back(std::move(current));
			}
		}

		elems = std::move(temp);
	} while (!elems.empty());

	return false;
}

void XamlHelper::CloseComboBoxPopup(XamlRoot const& root) {
	for (const Popup& popup : VisualTreeHelper::GetOpenPopupsForXamlRoot(root)) {
		if (IsComboBoxPopup(popup)) {
			popup.IsOpen(false);
			return;
		}
	}
}

void XamlHelper::UpdateThemeOfXamlPopups(XamlRoot const& root, ElementTheme theme) {
	for (const auto& popup : VisualTreeHelper::GetOpenPopupsForXamlRoot(root)) {
		FrameworkElement child = popup.Child().as<FrameworkElement>();
		child.RequestedTheme(theme);
		UpdateThemeOfTooltips(child, theme);
	}
}

void XamlHelper::UpdateThemeOfTooltips(DependencyObject const& root, ElementTheme theme) {
	if (Win32Helper::GetOSVersion().IsWin11()) {
		// Win11 中 Tooltip 自动适应主题
		return;
	}

	// 遍历 XAML 树
	std::vector<DependencyObject> elems{ root };
	do {
		std::vector<DependencyObject> temp;

		for (const DependencyObject& elem : elems) {
			const int count = VisualTreeHelper::GetChildrenCount(elem);
			for (int i = 0; i < count; ++i) {
				DependencyObject current = VisualTreeHelper::GetChild(elem, i);

				if (IInspectable tooltipContent = ToolTipService::GetToolTip(current)) {
					if (ToolTip tooltip = tooltipContent.try_as<ToolTip>()) {
						tooltip.RequestedTheme(theme);
					} else {
						ToolTip themedTooltip;
						themedTooltip.Content(tooltipContent);
						themedTooltip.RequestedTheme(theme);
						ToolTipService::SetToolTip(current, themedTooltip);
					}
				}

				temp.emplace_back(std::move(current));
			}
		}

		elems = std::move(temp);
	} while (!elems.empty());
}

}
