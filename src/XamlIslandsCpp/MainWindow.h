#pragma once
#include "XamlWindow.h"
#include <winrt/XamlIslandsCpp.App.h>

namespace XamlIslandsCpp {

class MainWindow : public XamlWindowT<MainWindow, winrt::XamlIslandsCpp::App::RootPage> {
	friend base_type;
public:
	bool Create(
		HINSTANCE hInstance,
		winrt::XamlIslandsCpp::App::AppTheme theme,
		winrt::XamlIslandsCpp::App::WindowBackdrop backdrop,
		bool isCustomTitleBarEnabled
	) noexcept;

	bool SetTheme(winrt::XamlIslandsCpp::App::AppTheme theme, winrt::XamlIslandsCpp::App::WindowBackdrop backdrop) noexcept;

	void SetCustomTitleBar(bool enabled) noexcept;

protected:
	LRESULT _MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

private:
	static LRESULT CALLBACK _TitleBarWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	LRESULT _TitleBarMessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	void _ResizeTitleBarWindow() noexcept;

	HWND _hwndTitleBar = NULL;
	HWND _hwndMaximizeButton = NULL;
	bool _trackingMouse = false;
};

}
