#include "pch.h"
#include "MainWindow.h"
#include <CoreWindow.h>
#include "Utils.h"

bool MainWindow::Initialize(HINSTANCE hInstance) noexcept {
	WNDCLASSEXW wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = _WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"XamlIslandsCpp";
	wcex.hIconSm = NULL;

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

	_SetContent(winrt::XamlIslandsCpp::App::MainPage());

	return true;
}

LRESULT MainWindow::_MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	switch (msg) {
	case WM_GETMINMAXINFO:
	{
		// 设置窗口最小尺寸
		MINMAXINFO* mmi = (MINMAXINFO*)lParam;
		mmi->ptMinTrackSize = { 500,300 };
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return base_type::_MessageHandler(msg, wParam, lParam);
}
