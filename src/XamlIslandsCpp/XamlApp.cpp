#include "pch.h"
#include "XamlApp.h"
#include "Utils.h"
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>
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

	// 初始化 UWP 应用
	_uwpApp = winrt::XamlIslandsCpp::App::App();

	_mainPage = winrt::XamlIslandsCpp::App::MainPage();
	// MainPage 加载完成后显示主窗口
	_mainPage.Loaded([this](winrt::IInspectable const&, winrt::RoutedEventArgs const&) -> winrt::IAsyncAction {
		co_await _mainPage.Dispatcher().RunAsync(winrt::CoreDispatcherPriority::Normal, [hwndXamlHost(_hwndXamlHost)]() {
			// 防止窗口显示时背景闪烁
			// https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit
			SetWindowPos(hwndXamlHost, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			ShowWindow(hwndXamlHost, SW_SHOWNORMAL);
		});
	});

	// 初始化 XAML Islands
	_xamlSource = winrt::DesktopWindowXamlSource();
	_xamlSourceNative2 = _xamlSource.as<IDesktopWindowXamlSourceNative2>();

	auto interop = _xamlSource.as<IDesktopWindowXamlSourceNative>();
	interop->AttachToWindow(_hwndXamlHost);
	interop->get_WindowHandle(&_hwndXamlIsland);
	_xamlSource.Content(_mainPage);

	// 防止第一次收到 WM_SIZE 消息时 MainPage 尺寸为 0
	_OnResize();

	// 焦点始终位于 _hwndXamlIsland 中
	_xamlSource.TakeFocusRequested([](winrt::DesktopWindowXamlSource const& sender, winrt::DesktopWindowXamlSourceTakeFocusRequestedEventArgs const& args) {
		winrt::XamlSourceFocusNavigationReason reason = args.Request().Reason();
		if (reason < winrt::XamlSourceFocusNavigationReason::Left) {
			sender.NavigateFocus(args.Request());
		}
	});

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

XamlApp::XamlApp() {}

XamlApp::~XamlApp() {}

void XamlApp::_OnResize() {
	RECT clientRect;
	GetClientRect(_hwndXamlHost, &clientRect);
	SetWindowPos(_hwndXamlIsland, NULL, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, SWP_SHOWWINDOW | SWP_NOACTIVATE);
}

LRESULT XamlApp::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_SHOWWINDOW:
	{
		if (wParam == TRUE) {
			SetFocus(_hwndXamlHost);
		}

		break;
	}
	case WM_KEYDOWN:
	{
		if (wParam == VK_TAB) {
			// 处理焦点
			if (_xamlSource) {
				winrt::XamlSourceFocusNavigationReason reason = (GetKeyState(VK_SHIFT) & 0x80) ?
					winrt::XamlSourceFocusNavigationReason::Last : winrt::XamlSourceFocusNavigationReason::First;
				_xamlSource.NavigateFocus(winrt::XamlSourceFocusNavigationRequest(reason));
			}
			return 0;
		}
		break;
	}
	case WM_SIZE:
	{
		if (wParam != SIZE_MINIMIZED) {
			_OnResize();
			if (_mainPage) {
				// 使 ContentDialog 跟随窗口尺寸调整
				// 来自 https://github.com/microsoft/microsoft-ui-xaml/issues/3577#issuecomment-1399250405
				if (winrt::CoreWindow coreWindow = winrt::CoreWindow::GetForCurrentThread()) {
					HWND hwndDWXS;
					coreWindow.as<ICoreWindowInterop>()->get_WindowHandle(&hwndDWXS);
					int width = LOWORD(lParam);
					int height = HIWORD(lParam);
					PostMessage(hwndDWXS, WM_SIZE, wParam, lParam);
				}

				[](XamlApp* app)->winrt::fire_and_forget {
					co_await app->_mainPage.Dispatcher().RunAsync(winrt::CoreDispatcherPriority::Normal, [app]() {
						Utils::RepositionXamlPopups(app->_mainPage.XamlRoot(), true);
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
	case WM_SYSCOMMAND:
	{
		// Alt 键默认会打开菜单，导致界面不响应鼠标移动。这里禁用这个行为
		if ((wParam & 0xfff0) == SC_KEYMENU) {
			return 0;
		}

		// 最小化时关闭 ComboBox
		// 不能在 WM_SIZE 中处理，该消息发送于最小化之后，会导致 ComboBox 无法交互
		if (wParam == SC_MINIMIZE && _mainPage) {
			Utils::CloseXamlPopups(_mainPage.XamlRoot());
		}
		
		break;
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
			Utils::RepositionXamlPopups(_mainPage.XamlRoot(), false);
		}

		return 0;
	}
	case WM_DPICHANGED:
	{
		RECT* newRect = (RECT*)lParam;
		SetWindowPos(hWnd,
			NULL,
			newRect->left,
			newRect->top,
			newRect->right - newRect->left,
			newRect->bottom - newRect->top,
			SWP_NOZORDER | SWP_NOACTIVATE
		);

		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
