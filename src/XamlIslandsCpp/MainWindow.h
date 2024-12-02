#pragma once
#include "XamlWindow.h"
#include "AppSettings.h"
#include <winrt/XamlIslandsCpp.h>

namespace XamlIslandsCpp {

class MainWindow : public XamlWindowT<MainWindow, winrt::XamlIslandsCpp::RootPage> {
	friend base_type;
public:
	MainWindow(const MainWindow&) = delete;
	MainWindow(MainWindow&&) = delete;

	static MainWindow& Get() {
		static MainWindow instance;
		return instance;
	}

	bool Create(HINSTANCE hInstance, const WINDOWPLACEMENT* wp = nullptr) noexcept;

protected:
	LRESULT _MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

private:
	MainWindow() = default;

	static LRESULT CALLBACK _TitleBarWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	LRESULT _TitleBarMessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	void _ResizeTitleBarWindow() noexcept;

	WinRTHelper::EventRevoker _themeChangedRevoker;
	WinRTHelper::EventRevoker _isCustomTitleBarEnabledChangedRevoker;
	WinRTHelper::EventRevoker _backdropChangedRevoker;

	HWND _hwndTitleBar = NULL;
	HWND _hwndMaximizeButton = NULL;
	bool _trackingMouse = false;

	// 防止重新创建主窗口时退出
	bool _closingForRecreate = false;
};

}
