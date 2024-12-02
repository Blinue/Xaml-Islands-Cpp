#pragma once
#include "XamlWindow.h"
#include "AppSettings.h"

namespace XamlIslandsCpp {

class MainWindow : public XamlWindowT<MainWindow, winrt::XamlIslandsCpp::RootPage> {
	friend base_type;
public:
	bool Create(
		HINSTANCE hInstance,
		AppTheme theme,
		WindowBackdrop backdrop,
		bool isCustomTitleBarEnabled,
		const WINDOWPLACEMENT* wp = nullptr
	) noexcept;

	bool SetTheme(AppTheme theme, WindowBackdrop backdrop) noexcept;

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
