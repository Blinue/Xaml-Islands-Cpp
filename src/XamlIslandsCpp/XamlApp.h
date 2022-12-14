#pragma once
#include "pch.h"
#include <winrt/XamlIslandsCpp.App.h>

struct IDesktopWindowXamlSourceNative2;

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
	XamlApp();
	~XamlApp();

	void _OnResize();

	static LRESULT _WndProcStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		return Get()._WndProc(hWnd, msg, wParam, lParam);
	}
	LRESULT _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	winrt::XamlIslandsCpp::App::App _uwpApp{ nullptr };
	winrt::XamlIslandsCpp::App::MainPage _mainPage{ nullptr };
	HWND _hwndXamlHost = NULL;
	HWND _hwndXamlIsland = NULL;

	winrt::DesktopWindowXamlSource _xamlSource{ nullptr };
	winrt::com_ptr<IDesktopWindowXamlSourceNative2> _xamlSourceNative2;
};
