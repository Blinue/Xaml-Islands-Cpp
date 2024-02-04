#include "pch.h"
#include "MainWindow.h"
#include <CoreWindow.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "UxTheme.lib")

bool MainWindow::Initialize(HINSTANCE hInstance) noexcept {
	WNDCLASSEXW wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = _WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.lpszClassName = L"XamlIslandsCpp";

	RegisterClassEx(&wcex);

	CreateWindow(
		L"XamlIslandsCpp",
		L"XamlIslandsCpp",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr,
		nullptr,
		hInstance,
		this
	);

	if (!_hWnd) {
		return false;
	}

	auto a = SetWindowLongPtr(_hWnd, GWL_EXSTYLE, WS_EX_NOREDIRECTIONBITMAP);
	SetWindowPos(_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

	// 隐藏原生标题栏上的图标
	SetWindowThemeNonClientAttributes(_hWnd, WTNCA_NODRAWICON | WTNCA_NOSYSMENU, WTNCA_VALIDBITS);

	/*BOOL value1 = TRUE;
	DwmSetWindowAttribute(
		_hWnd,
		DWMWA_USE_IMMERSIVE_DARK_MODE,
		&value1,
		sizeof(value1)
	);*/
	

	DWM_SYSTEMBACKDROP_TYPE value = DWMSBT_NONE;
	DwmSetWindowAttribute(_hWnd, DWMWA_SYSTEMBACKDROP_TYPE, &value, sizeof(value));

	_SetContent(winrt::XamlIslandsCpp::App::RootPage());

	return true;
}

LRESULT MainWindow::_MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	switch (msg) {
	case WM_GETMINMAXINFO:
	{
		// 设置窗口最小尺寸
		MINMAXINFO* mmi = (MINMAXINFO*)lParam;
		mmi->ptMinTrackSize = { 600,400 };
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return base_type::_MessageHandler(msg, wParam, lParam);
}
