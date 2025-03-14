#pragma once
#include "XamlWindow.h"
#include "RootPage.h"

namespace XamlIslandsCpp {

class MainWindow : public XamlWindowT<MainWindow, winrt::com_ptr<winrt::XamlIslandsCpp::implementation::RootPage>> {
	using base_type = XamlWindowT<MainWindow, winrt::com_ptr<winrt::XamlIslandsCpp::implementation::RootPage>>;
	friend base_type;

public:
	bool Create(const WINDOWPLACEMENT* wp = nullptr) noexcept;

protected:
	LRESULT _MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

private:
	static LRESULT CALLBACK _TitleBarWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	LRESULT _TitleBarMessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	void _ResizeTitleBarWindow() noexcept;

	Event<bool>::EventRevoker _appThemeChangedRevoker;
	Event<bool>::EventRevoker _isCustomTitleBarEnabledChangedRevoker;
	Event<WindowBackdrop>::EventRevoker _backdropChangedRevoker;

	HWND _hwndTitleBar = NULL;
	HWND _hwndMaximizeButton = NULL;
	bool _trackingMouse = false;

	// 防止重新创建主窗口时退出
	bool _closingForRecreate = false;
	bool _smoothResizedEnabled = false;
};

}
