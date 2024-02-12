#include "pch.h"
#include "MainWindow.h"
#include <CoreWindow.h>
#include "CommonSharedConstants.h"

#pragma comment(lib, "UxTheme.lib")

namespace XamlIslandsCpp {

bool MainWindow::Create(HINSTANCE hInstance) noexcept {
	static const int _ = [](HINSTANCE hInstance) {
		WNDCLASSEXW wcex{
			.cbSize = sizeof(WNDCLASSEX),
			.lpfnWndProc = _WndProc,
			.hInstance = hInstance,
			.hCursor = LoadCursor(nullptr, IDC_ARROW),
			.lpszClassName = CommonSharedConstants::MAIN_WINDOW_CLASS_NAME
		};
		RegisterClassEx(&wcex);

		return 0;
	}(hInstance);

	CreateWindowEx(
		Win32Helper::GetOSVersion().Is22H2OrNewer() ? WS_EX_NOREDIRECTIONBITMAP : 0,
		CommonSharedConstants::MAIN_WINDOW_CLASS_NAME,
		L"XamlIslandsCpp",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr,
		nullptr,
		hInstance,
		this
	);
	assert(_hWnd);

	// 隐藏原生标题栏上的图标
	SetWindowThemeNonClientAttributes(_hWnd, WTNCA_NODRAWICON | WTNCA_NOSYSMENU, WTNCA_VALIDBITS);

	_SetContent(winrt::XamlIslandsCpp::App::RootPage());

	_UpdateTheme(false);

	// 1. 刷新窗口边框
	// 2. 防止窗口显示时背景闪烁: https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit
	SetWindowPos(_hWnd, NULL, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOCOPYBITS);

	// Xaml 控件加载完成后显示主窗口
	_content.Loaded([this](winrt::IInspectable const&, winrt::RoutedEventArgs const&) {
		ShowWindow(_hWnd, SW_SHOWNORMAL);
	});

	// 创建标题栏窗口，它是主窗口的子窗口。我们将它置于 XAML Islands 窗口之上以防止鼠标事件被吞掉
	// 
	// 出于未知的原因，必须添加 WS_EX_LAYERED 样式才能发挥作用，见
	// https://github.com/microsoft/terminal/blob/0ee2c74cd432eda153f3f3e77588164cde95044f/src/cascadia/WindowsTerminal/NonClientIslandWindow.cpp#L79
	// WS_EX_NOREDIRECTIONBITMAP 可以避免 WS_EX_LAYERED 导致的额外内存开销
	//
	// WS_MINIMIZEBOX 和 WS_MAXIMIZEBOX 使得鼠标悬停时显示文字提示，Win11 的贴靠布局不依赖它们
	CreateWindowEx(
		WS_EX_LAYERED | WS_EX_NOPARENTNOTIFY | WS_EX_NOREDIRECTIONBITMAP | WS_EX_NOACTIVATE,
		CommonSharedConstants::TITLE_BAR_WINDOW_CLASS_NAME,
		L"",
		WS_CHILD | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		0, 0, 0, 0,
		_hWnd,
		nullptr,
		hInstance,
		this
	);
	SetLayeredWindowAttributes(_hwndTitleBar, 0, 255, LWA_ALPHA);

	if (Win32Helper::GetOSVersion().IsWin11()) {
		// 如果鼠标正位于一个按钮上，贴靠布局弹窗会出现在按钮下方。我们利用这个特性来修正贴靠布局弹窗的位置
		_hwndMaximizeButton = CreateWindow(
			L"BUTTON",
			L"",
			WS_VISIBLE | WS_CHILD | WS_DISABLED | BS_OWNERDRAW,
			0, 0, 0, 0,
			_hwndTitleBar,
			NULL,
			hInstance,
			NULL
		);
	}

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
	{
		_hwndTitleBar = NULL;
		_trackingMouse = false;

		PostQuitMessage(0);
		break;
	}
	}
	return base_type::_MessageHandler(msg, wParam, lParam);
}

void MainWindow::_UpdateTheme(bool isDarkTheme) noexcept {
	XamlWindowT::_SetTheme(isDarkTheme);
}

}
