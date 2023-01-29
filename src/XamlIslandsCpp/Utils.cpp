#include "pch.h"
#include "Utils.h"

using namespace winrt;

UINT Utils::GetOSBuild() {
	static UINT build = 0;

	if (build == 0) {
		HMODULE hNtDll = GetModuleHandle(L"ntdll.dll");
		if (!hNtDll) {
			return {};
		}

		auto rtlGetVersion = (LONG(WINAPI*)(PRTL_OSVERSIONINFOW))GetProcAddress(hNtDll, "RtlGetVersion");
		if (rtlGetVersion == nullptr) {
			return {};
		}

		OSVERSIONINFOW version{};
		version.dwOSVersionInfoSize = sizeof(version);
		rtlGetVersion(&version);

		build = version.dwBuildNumber;
	}

	return build;
}

void Utils::CloseXamlPopups(XamlRoot const& root) {
	if (!root) {
		return;
	}

	for (const auto& popup : VisualTreeHelper::GetOpenPopupsForXamlRoot(root)) {
		hstring className = get_class_name(popup.Child());
		if (className == name_of<Controls::ContentDialog>() || className == name_of<Shapes::Rectangle>()) {
			continue;
		}

		popup.IsOpen(false);
	}
}

void Utils::RepositionXamlPopups(XamlRoot const& root, bool closeFlyoutPresenter) {
	if (!root) {
		return;
	}

	for (const auto& popup : VisualTreeHelper::GetOpenPopupsForXamlRoot(root)) {
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
		if (compositeMode == ElementCompositeMode::SourceOver) {
			popup.CompositeMode(ElementCompositeMode::MinBlend);
		} else {
			popup.CompositeMode(ElementCompositeMode::SourceOver);
		}

		// Restore CompositeMode to whatever it was originally set to.
		popup.CompositeMode(compositeMode);
	}
}
