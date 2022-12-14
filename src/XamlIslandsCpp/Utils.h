#pragma once

struct Utils {
	static UINT GetOSBuild();
	static void CloseXamlPopups(winrt::XamlRoot const& root);
	static void RepositionXamlPopups(winrt::XamlRoot const& root, bool closeFlyoutPresenter);
	static void ResizeContentDialog(winrt::XamlRoot const& root);
};

