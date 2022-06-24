#include "pch.h"
#include "Utils.h"


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

void Utils::CloseXamlPopups(winrt::XamlRoot const& root) {
	if (!root) {
		return;
	}

	for (const auto& popup : winrt::VisualTreeHelper::GetOpenPopupsForXamlRoot(root)) {
		if (!popup.IsConstrainedToRootBounds()) {
			popup.IsOpen(false);
		}
	}
}

void Utils::RepositionXamlPopups(winrt::XamlRoot const& root) {
	if (!root) {
		return;
	}

	for (const auto& popup : winrt::VisualTreeHelper::GetOpenPopupsForXamlRoot(root)) {
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

// 使 ContentDialog 跟随窗口尺寸调整
void Utils::ResizeXamlDialog(winrt::XamlRoot const& root) {
	if (!root) {
		return;
	}

	winrt::Size rootSize = root.Size();

	for (const auto& popup : winrt::VisualTreeHelper::GetOpenPopupsForXamlRoot(root)) {
		if (!popup.IsConstrainedToRootBounds()) {
			continue;
		}

		winrt::FrameworkElement child = popup.Child().as<winrt::FrameworkElement>();
		child.Width(rootSize.Width);
		child.Height(rootSize.Height);
	}
}
