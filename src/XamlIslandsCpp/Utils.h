#pragma once

struct Utils {
	static UINT GetOSBuild();
	static void RepositionXamlPopups(winrt::XamlRoot const& root, bool closeFlyoutPresenter);
};

