#include "pch.h"
#include "XamlApp.h"
#include "Utils.h"
#include <CoreWindow.h>


bool XamlApp::Initialize(HINSTANCE hInstance) {
	// 注册窗口类
	{
		WNDCLASSEXW wcex{};

		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.lpfnWndProc = _WndProcStatic;
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
	}

	_hwndXamlHost = CreateWindow(
		L"XamlIslandsCpp",
		L"XamlIslandsCpp",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	if (!_hwndXamlHost) {
		return false;
	}

	bool isWin11 = Utils::GetOSBuild() >= 22000;

	// 防止窗口显示时背景闪烁
	// https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit
	SetWindowPos(_hwndXamlHost, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	if (!isWin11) {
		// Win10 中任务栏可能出现空的 DesktopWindowXamlSource 窗口
		// 见 https://github.com/microsoft/microsoft-ui-xaml/issues/6490
		// 如果不将 ShowWindow 提前，任务栏会短暂出现两个图标
		ShowWindow(_hwndXamlHost, SW_SHOW);
	}

	// 初始化 UWP 应用和 XAML Islands
	_uwpApp = winrt::XamlIslandsCpp::App::App();
	if (!isWin11) {
		// 隐藏 DesktopWindowXamlSource 窗口
		winrt::CoreWindow coreWindow = winrt::CoreWindow::GetForCurrentThread();
		if (coreWindow) {
			HWND hwndDWXS;
			coreWindow.as<ICoreWindowInterop>()->get_WindowHandle(&hwndDWXS);
			ShowWindow(hwndDWXS, SW_HIDE);
		}
	}

	_mainPage = winrt::XamlIslandsCpp::App::MainPage();
	if (isWin11) {
		// Win11 中在 MainPage 加载完成后再显示主窗口
		_mainPage.Loaded([this](winrt::IInspectable const&, winrt::RoutedEventArgs const&) {
			ShowWindow(_hwndXamlHost, SW_SHOW);
			SetFocus(_hwndXamlHost);
		});
	}

	_xamlSource = winrt::DesktopWindowXamlSource();
	_xamlSourceNative2 = _xamlSource.as<IDesktopWindowXamlSourceNative2>();

	auto interop = _xamlSource.as<IDesktopWindowXamlSourceNative>();
	interop->AttachToWindow(_hwndXamlHost);
	interop->get_WindowHandle(&_hwndXamlIsland);
	_xamlSource.Content(_mainPage);

	// 焦点始终位于 _hwndXamlIsland 中
	_xamlSource.TakeFocusRequested([](winrt::DesktopWindowXamlSource const& sender, winrt::DesktopWindowXamlSourceTakeFocusRequestedEventArgs const& args) {
		winrt::XamlSourceFocusNavigationReason reason = args.Request().Reason();
		if (reason < winrt::XamlSourceFocusNavigationReason::Left) {
			sender.NavigateFocus(args.Request());
		}
	});

	if (!isWin11) {
		// Win10 中因为 ShowWindow 提前，需要显式设置 XAML Islands 位置
		_OnResize();
	}

	return true;
}

int XamlApp::Run() {
	MSG msg;

	// 主消息循环
	while (GetMessage(&msg, nullptr, 0, 0)) {
		BOOL processed = FALSE;
		HRESULT hr = _xamlSourceNative2->PreTranslateMessage(&msg, &processed);
		if (SUCCEEDED(hr) && processed) {
			continue;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	_xamlSourceNative2 = nullptr;
	_xamlSource.Close();
	_xamlSource = nullptr;
	_mainPage = nullptr;
	_uwpApp = nullptr;

	return (int)msg.wParam;
}

void XamlApp::_OnResize() {
	RECT clientRect;
	GetClientRect(_hwndXamlHost, &clientRect);
	SetWindowPos(_hwndXamlIsland, NULL, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, SWP_SHOWWINDOW | SWP_NOACTIVATE);
}

LRESULT XamlApp::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_KEYDOWN:
	{
		if (wParam == VK_TAB) {
			// 处理焦点
			if (_xamlSource) {
				BYTE keyboardState[256] = {};
				if (GetKeyboardState(keyboardState)) {
					winrt::XamlSourceFocusNavigationReason reason = (keyboardState[VK_SHIFT] & 0x80) ?
						winrt::XamlSourceFocusNavigationReason::Last : winrt::XamlSourceFocusNavigationReason::First;
					_xamlSource.NavigateFocus(winrt::XamlSourceFocusNavigationRequest(reason));
				}
			}
			return 0;
		}
		break;
	}
	case WM_SIZE:
	{
		if (wParam == SIZE_MINIMIZED) {
			if (_mainPage) {
				Utils::CloseXamlPopups(_mainPage.XamlRoot());
			}			
		} else {
			_OnResize();
			if (_mainPage) {
				[](XamlApp* app)->winrt::fire_and_forget {
					co_await app->_mainPage.Dispatcher().RunAsync(winrt::CoreDispatcherPriority::Normal, [app]() {
						Utils::ResizeXamlDialog(app->_mainPage.XamlRoot());
						Utils::RepositionXamlPopups(app->_mainPage.XamlRoot());
					});
				}(this);
			}
		}
		return 0;
	}
	case WM_GETMINMAXINFO:
	{
		// 设置窗口最小尺寸
		MINMAXINFO* mmi = (MINMAXINFO*)lParam;
		mmi->ptMinTrackSize = { 500,300 };
		return 0;
	}
	case WM_ACTIVATE:
	{
		if (_uwpApp) {
			if (LOWORD(wParam) != WA_INACTIVE) {
				SetFocus(_hwndXamlIsland);
			} else {
				Utils::CloseXamlPopups(_mainPage.XamlRoot());
			}
		}
		
		return 0;
	}
	case WM_MOVING:
	{
		if (_mainPage) {
			Utils::RepositionXamlPopups(_mainPage.XamlRoot());
		}

		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
