#pragma once
#include "XamlWindow.h"
#include <winrt/XamlIslandsCpp.App.h>

namespace XamlIslandsCpp {

class MainWindow : public XamlWindowT<MainWindow, winrt::XamlIslandsCpp::App::RootPage> {
	friend base_type;
public:
	bool Create(HINSTANCE hInstance) noexcept;

protected:
	LRESULT _MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

private:
	void _UpdateTheme(bool isDarkTheme) noexcept;

	HWND _hwndTitleBar = NULL;
	HWND _hwndMaximizeButton = NULL;
	bool _trackingMouse = false;
};

}
