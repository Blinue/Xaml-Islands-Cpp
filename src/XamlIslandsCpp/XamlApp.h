#pragma once
#include "pch.h"
#include "MainWindow.h"

namespace XamlIslandsCpp {

class XamlApp {
public:
	XamlApp(const XamlApp&) = delete;
	XamlApp(XamlApp&&) = delete;

	static XamlApp& Get() {
		static XamlApp instance;
		return instance;
	}

	bool Initialize(HINSTANCE hInstance);

	int Run();

private:
	XamlApp() = default;

	bool _CreateMainWindow(HINSTANCE hInstance) noexcept;

	void _MainWindow_Destoryed();

	winrt::XamlIslandsCpp::App::App _uwpApp{ nullptr };
	MainWindow _mainWindow;

	winrt::XamlIslandsCpp::App::Settings::ThemeChanged_revoker _themeChangedRevoker;
	winrt::XamlIslandsCpp::App::Settings::IsCustomTitleBarEnabledChanged_revoker _isCustomTitleBarEnabledChangedRevoker;
};

}
