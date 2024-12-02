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

	bool _CreateMainWindow(const WINDOWPLACEMENT* wp = nullptr) noexcept;

	void _MainWindow_Destoryed();

	HINSTANCE _hInstance = NULL;
	winrt::XamlIslandsCpp::App _uwpApp{ nullptr };
	MainWindow _mainWindow;

	WinRTHelper::EventRevoker _themeChangedRevoker;
	WinRTHelper::EventRevoker _isCustomTitleBarEnabledChangedRevoker;
	WinRTHelper::EventRevoker _backdropChangedRevoker;

	// 防止重新创建主窗口时退出
	bool shouldQuit = true;
};

}
