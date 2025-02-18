#include "pch.h"
#include "MainWindow.h"
#include "CommonSharedConstants.h"
#include "CaptionButtonsControl.h"
#include "TitleBarControl.h"
#include "Win32Helper.h"
#include "App.h"

using namespace winrt::XamlIslandsCpp::implementation;
using namespace winrt;

namespace XamlIslandsCpp {

bool MainWindow::Create(const WINDOWPLACEMENT* wp) noexcept {
	static const int _ = []() {
		const HINSTANCE hInstance = Win32Helper::GetModuleInstanceHandle();

		WNDCLASSEXW wcex{
			.cbSize = sizeof(WNDCLASSEX),
			.lpfnWndProc = _WndProc,
			.hInstance = hInstance,
			.hCursor = LoadCursor(nullptr, IDC_ARROW),
			.lpszClassName = CommonSharedConstants::MAIN_WINDOW_CLASS_NAME
		};
		RegisterClassEx(&wcex);

		wcex.style = CS_DBLCLKS;
		wcex.lpfnWndProc = _TitleBarWndProc;
		wcex.hIcon = NULL;
		wcex.lpszClassName = CommonSharedConstants::TITLE_BAR_WINDOW_CLASS_NAME;
		RegisterClassEx(&wcex);

		return 0;
	}();

	const bool isLightTheme = App::Get().IsLightTheme();

	AppSettings& settings = AppSettings::Get();
	const WindowBackdrop backdrop = settings.Backdrop();

	_SetInitialTheme(isLightTheme, backdrop, settings.IsCustomTitleBarEnabled());

	const HINSTANCE hInstance = Win32Helper::GetModuleInstanceHandle();

	CreateWindowEx(
		Win32Helper::GetOSVersion().Is22H2OrNewer() && backdrop != WindowBackdrop::SolidColor ? WS_EX_NOREDIRECTIONBITMAP : 0,
		CommonSharedConstants::MAIN_WINDOW_CLASS_NAME,
		L"XamlIslandsCpp",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		nullptr,
		nullptr,
		hInstance,
		this
	);
	assert(Handle());

	_Content(make_self<RootPage>());

	_appThemeChangedRevoker = App::Get().ThemeChanged(winrt::auto_revoke,
		[this](bool isLightTheme) { _SetTheme(isLightTheme, AppSettings::Get().Backdrop()); });
	_SetTheme(isLightTheme, backdrop);

	// 隐藏原生标题栏上的图标
	SetWindowThemeNonClientAttributes(Handle(), WTNCA_NODRAWICON | WTNCA_NOSYSMENU, WTNCA_VALIDBITS);

	// 1. 刷新窗口边框
	// 2. 防止窗口显示时背景闪烁: https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit
	SetWindowPos(Handle(), NULL, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOCOPYBITS);

	// Xaml 控件加载完成后显示主窗口
	if (wp) {
		Content()->Loaded([this, wp(*wp)](winrt::IInspectable const&, RoutedEventArgs const&) {
			// 禁用显示窗口的动画
			BOOL value = TRUE;
			DwmSetWindowAttribute(Handle(), DWMWA_TRANSITIONS_FORCEDISABLED, &value, sizeof(value));

			SetWindowPlacement(Handle(), &wp);

			value = FALSE;
			DwmSetWindowAttribute(Handle(), DWMWA_TRANSITIONS_FORCEDISABLED, &value, sizeof(value));
		});
	} else {
		Content()->Loaded([this](winrt::IInspectable const&, winrt::RoutedEventArgs const&) {
			ShowWindow(Handle(), SW_SHOWNORMAL);
		});
	}

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
		Handle(),
		nullptr,
		hInstance,
		this
	);
	SetLayeredWindowAttributes(_hwndTitleBar, 0, 255, LWA_ALPHA);

	if (Win32Helper::GetOSVersion().IsWin11()) {
		// 如果鼠标正位于一个按钮上，贴靠布局弹窗会出现在按钮下方。我们利用这个特性来修正贴靠布局弹窗的位置。
		// Win11 23H2 的某一次更新后，Snap Layout 不再依赖 UI Automation，而是依靠 WM_GETTITLEBARINFOEX
        // 消息来定位最大化按钮矩形。此行为破坏了许多程序的 Snap Layout 支持，好在 Win11 24H2 中问题得到了
		// 缓解。我们同时支持两种方案，以便在不同版本的 Win11 上都能正常工作。
		_hwndMaximizeButton = CreateWindowEx(
			WS_EX_NOPARENTNOTIFY,
			L"BUTTON",
			L"",
			WS_VISIBLE | WS_CHILD | WS_DISABLED | BS_OWNERDRAW,
			0, 0, 0, 0,
			_hwndTitleBar,
			NULL,
			hInstance,
			NULL
		);

		// 允许 WM_GETTITLEBARINFOEX 通过 UIPI 防止以管理员身份运行时无法收到
		ChangeWindowMessageFilterEx(Handle(), WM_GETTITLEBARINFOEX, MSGFLT_ALLOW, nullptr);
	}

	Content()->TitleBar().SizeChanged([this](winrt::IInspectable const&, SizeChangedEventArgs const&) {
		_ResizeTitleBarWindow();
	});

	_backdropChangedRevoker = settings.BackdropChanged(
		auto_revoke,
		[this](WindowBackdrop backdrop) {
			if (!_SetTheme(App::Get().IsLightTheme(), backdrop)) {
				return;
			}

			// 由于无法更改 WS_EX_NOREDIRECTIONBITMAP 样式，必须重新创建主窗口。
			// 应延迟执行，让 UI 完成更新。
			App::Get().Dispatcher().TryRunAsync(CoreDispatcherPriority::Normal, [this]() {
				WINDOWPLACEMENT wp{ .length = sizeof(wp) };
				GetWindowPlacement(Handle(), &wp);

				// 禁用关闭窗口的动画
				BOOL value = TRUE;
				DwmSetWindowAttribute(Handle(), DWMWA_TRANSITIONS_FORCEDISABLED, &value, sizeof(value));

				_closingForRecreate = true;
				Destroy();
				_closingForRecreate = false;

				Create(&wp);
			});
		}
	);

	_isCustomTitleBarEnabledChangedRevoker = settings.IsCustomTitleBarEnabledChanged(
		auto_revoke,
		[&](bool enabled) {
			if (_IsCustomTitleBarEnabled() == enabled) {
				return;
			}

			ShowWindow(_hwndTitleBar, enabled ? SW_SHOW : SW_HIDE);

			if (enabled) {
				_SetCustomTitleBar(true);
			} else {
				// 优化动画
				Content()->Dispatcher().TryRunAsync(CoreDispatcherPriority::Normal, [this]() -> fire_and_forget {
					MainWindow* that = this;
					CoreDispatcher dispatcher = Content()->Dispatcher();
					co_await 10ms;
					co_await dispatcher;
					that->_SetCustomTitleBar(false);
				});
			}
		}
	);

	return true;
}

LRESULT MainWindow::_MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	switch (msg) {
	case WM_SIZE:
	{
		LRESULT ret = base_type::_MessageHandler(WM_SIZE, wParam, lParam);

		if (_IsCustomTitleBarEnabled()) {
			_ResizeTitleBarWindow();
			Content()->TitleBar().CaptionButtons().IsWindowMaximized(_IsMaximized());
		}
		
		return ret;
	}
	case WM_GETMINMAXINFO:
	{
		// 设置窗口最小尺寸
		const double dpiScale = _CurrentDpi() / double(USER_DEFAULT_SCREEN_DPI);
		((MINMAXINFO*)lParam)->ptMinTrackSize = {
			std::lround(500 * dpiScale),
			std::lround(420 * dpiScale)
		};
		return 0;
	}
	case WM_NCRBUTTONUP:
	{
		if (_IsCustomTitleBarEnabled() && wParam == HTCAPTION) {
			// 我们自己处理标题栏右键，不知为何 DefWindowProc 没有作用
			const POINT cursorPt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

			// 在标题栏上按下右键，在其他地方释放也会收到此消息。确保只有在标题栏上释放时才显示菜单
			RECT titleBarRect;
			GetWindowRect(_hwndTitleBar, &titleBarRect);
			if (!PtInRect(&titleBarRect, cursorPt)) {
				break;
			}

			HMENU systemMenu = GetSystemMenu(Handle(), FALSE);

			// 根据窗口状态更新选项
			auto setState = [&](UINT item, bool enabled) {
				MENUITEMINFO mii{
					.cbSize = sizeof(MENUITEMINFO),
					.fMask = MIIM_STATE,
					.fType = MFT_STRING,
					.fState = UINT(enabled ? MF_ENABLED : MF_DISABLED)
				};
				SetMenuItemInfo(systemMenu, item, FALSE, &mii);
			};
			const bool isMaximized = _IsMaximized();
			setState(SC_RESTORE, isMaximized);
			setState(SC_MOVE, !isMaximized);
			setState(SC_SIZE, !isMaximized);
			setState(SC_MINIMIZE, true);
			setState(SC_MAXIMIZE, !isMaximized);
			setState(SC_CLOSE, true);
			SetMenuDefaultItem(systemMenu, UINT_MAX, FALSE);

			BOOL cmd = TrackPopupMenu(systemMenu, TPM_RETURNCMD, cursorPt.x, cursorPt.y, 0, Handle(), nullptr);
			if (cmd != 0) {
				PostMessage(Handle(), WM_SYSCOMMAND, cmd, 0);
			}
		}

		break;
	}
	case WM_ACTIVATE:
	{
		if (_IsCustomTitleBarEnabled() && Content()) {
			Content()->TitleBar().IsWindowActive(LOWORD(wParam) != WA_INACTIVE);
		}
		break;
	}
	case WM_GETTITLEBARINFOEX:
	{
		if (_IsCustomTitleBarEnabled()) {
			// 为了支持 Win11 的贴靠布局，我们需要返回最大化按钮的矩形
			TITLEBARINFOEX* info = (TITLEBARINFOEX*)lParam;
			if (info->cbSize >= sizeof(TITLEBARINFOEX)) {
				base_type::_MessageHandler(msg, wParam, lParam);
				GetWindowRect(_hwndMaximizeButton, &info->rgrect[3]);
				return TRUE;
			}
		}
		break;
	}
	case WM_NCHITTEST:
	{
		// 为了和第三方程序兼容，确保主窗口本身可以正确响应 WM_NCHITTEST。
		// 见 https://github.com/microsoft/terminal/issues/8795
		if (_IsCustomTitleBarEnabled() && _hwndTitleBar) {
			// 自行处理标题栏区域，剩下的交给 OS
			LRESULT ht = _TitleBarMessageHandler(WM_NCHITTEST, 0, lParam);
			if (ht != HTNOWHERE) {
				return ht;
			}
		}
		break;
	}
	case WM_DESTROY:
	{
		_appThemeChangedRevoker.Revoke();
		_isCustomTitleBarEnabledChangedRevoker.Revoke();
		_backdropChangedRevoker.Revoke();
		
		// 标题栏窗口经常使用 Content()，确保在关闭 DWXS 前销毁
		DestroyWindow(_hwndTitleBar);
		_hwndTitleBar = NULL;
		_trackingMouse = false;

		if (!_closingForRecreate) {
			LRESULT ret = base_type::_MessageHandler(msg, wParam, lParam);
			// 由于基类会清空消息队列，PostQuitMessage 应在基类处理完毕后执行
			PostQuitMessage(0);
			return ret;
		}

		break;
	}
	}
	return base_type::_MessageHandler(msg, wParam, lParam);
}

LRESULT MainWindow::_TitleBarWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	if (msg == WM_NCCREATE) {
		MainWindow* that = (MainWindow*)(((CREATESTRUCT*)lParam)->lpCreateParams);
		assert(that && !that->_hwndTitleBar);
		that->_hwndTitleBar = hWnd;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)that);
	} else if (MainWindow* that = (MainWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA)) {
		return that->_TitleBarMessageHandler(msg, wParam, lParam);
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT MainWindow::_TitleBarMessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	switch (msg) {
	case WM_CTLCOLORBTN:
	{
		// 使原生按钮控件透明，虽然整个标题栏窗口都是不可见的
		return NULL;
	}
	case WM_NCHITTEST:
	{
		POINT cursorPos{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
		ScreenToClient(_hwndTitleBar, &cursorPos);

		RECT titleBarClientRect;
		GetClientRect(_hwndTitleBar, &titleBarClientRect);
		if (!PtInRect(&titleBarClientRect, cursorPos)) {
			// 先检查鼠标是否在窗口内。在标题栏按钮上按下鼠标时我们会捕获光标，从而收到 WM_MOUSEMOVE 和 WM_LBUTTONUP 消息。
			// 它们使用 WM_NCHITTEST 测试鼠标位于哪个区域
			return HTNOWHERE;
		}

		if (!_IsMaximized()) {
			const int resizeHandleHeight = _GetResizeHandleHeight();
			if (cursorPos.y < resizeHandleHeight) {
				// 鼠标位于上边框
				if (cursorPos.x < resizeHandleHeight) {
					return HTTOPLEFT;
				} else if (cursorPos.x + resizeHandleHeight >= titleBarClientRect.right) {
					return HTTOPRIGHT;
				} else {
					return HTTOP;
				}
			}
		}

		static const Size buttonSizeInDips = [this]() {
			return Content()->TitleBar().CaptionButtons().CaptionButtonSize();
		}();

		const double dpiScale = _CurrentDpi() / double(USER_DEFAULT_SCREEN_DPI);
		const double buttonWidthInPixels = buttonSizeInDips.Width * dpiScale;
		const double buttonHeightInPixels = buttonSizeInDips.Height * dpiScale;

		if (cursorPos.y >= _GetTopBorderThickness() + buttonHeightInPixels) {
			// 鼠标位于标题按钮下方，如果标题栏很宽，这里也可以拖动
			return HTCAPTION;
		}

		// 从右向左检查鼠标是否位于某个标题栏按钮上
		const LONG cursorToRight = titleBarClientRect.right - cursorPos.x;
		if (cursorToRight < buttonWidthInPixels) {
			return HTCLOSE;
		} else if (cursorToRight < buttonWidthInPixels * 2) {
			// 支持 Win11 的贴靠布局
			return HTMAXBUTTON;
		} else if (cursorToRight < buttonWidthInPixels * 3) {
			return HTMINBUTTON;
		} else {
			// 不在任何标题栏按钮上则在可拖拽区域
			return HTCAPTION;
		}
	}
	// 在捕获光标时会收到
	case WM_MOUSEMOVE:
	{
		POINT cursorPos{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
		ClientToScreen(_hwndTitleBar, &cursorPos);
		wParam = _TitleBarMessageHandler(WM_NCHITTEST, 0, MAKELPARAM(cursorPos.x, cursorPos.y));
		[[fallthrough]];
	}
	case WM_NCMOUSEMOVE:
	{
		CaptionButtonsControl& captionButtons = Content()->TitleBar().CaptionButtons();

		// 将 hover 状态通知 CaptionButtons。标题栏窗口拦截了 XAML Islands 中的标题栏
		// 控件的鼠标消息，标题栏按钮的状态由我们手动控制。
		switch (wParam) {
		case HTTOP:
		case HTTOPLEFT:
		case HTTOPRIGHT:
		case HTCAPTION:
		{
			captionButtons.LeaveButtons();

			// 将这些消息传给主窗口才能移动窗口或者调整窗口大小
			return _MessageHandler(msg, wParam, lParam);
		}
		case HTMINBUTTON:
		case HTMAXBUTTON:
		case HTCLOSE:
			captionButtons.HoverButton((CaptionButton)wParam);

			// 追踪鼠标以确保鼠标离开标题栏时我们能收到 WM_NCMOUSELEAVE 消息，否则无法
			// 可靠的收到这个消息，尤其是在用户快速移动鼠标的时候。
			if (!_trackingMouse && msg == WM_NCMOUSEMOVE) {
				TRACKMOUSEEVENT ev{
					.cbSize = sizeof(TRACKMOUSEEVENT),
					.dwFlags = TME_LEAVE | TME_NONCLIENT,
					.hwndTrack = _hwndTitleBar,
					.dwHoverTime = HOVER_DEFAULT // 不关心 HOVER 消息
				};
				TrackMouseEvent(&ev);
				_trackingMouse = true;
			}

			break;
		default:
			captionButtons.LeaveButtons();
		}
		break;
	}
	case WM_NCMOUSELEAVE:
	case WM_MOUSELEAVE:
	{
		// 我们需要检查鼠标是否**真的**离开了标题栏按钮，因为在某些情况下 OS 会错误汇报。
		// 比如：鼠标在关闭按钮上停留了一段时间，系统会显示文字提示，这时按下左键，便会收
		// 到 WM_NCMOUSELEAVE，但此时鼠标并没有离开标题栏按钮
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		// 先检查鼠标是否在主窗口上，如果正在显示文字提示，会返回 _hwndTitleBar
		HWND hwndUnderCursor = WindowFromPoint(cursorPos);
		if (hwndUnderCursor != Handle() && hwndUnderCursor != _hwndTitleBar) {
			Content()->TitleBar().CaptionButtons().LeaveButtons();
		} else {
			// 然后检查鼠标在标题栏上的位置
			LRESULT hit = _TitleBarMessageHandler(WM_NCHITTEST, 0, MAKELPARAM(cursorPos.x, cursorPos.y));
			if (hit != HTMINBUTTON && hit != HTMAXBUTTON && hit != HTCLOSE) {
				Content()->TitleBar().CaptionButtons().LeaveButtons();
			}
		}

		_trackingMouse = false;
		break;
	}
	case WM_NCLBUTTONDOWN:
	case WM_NCLBUTTONDBLCLK:
	{
		// 手动处理标题栏上的点击。如果在标题栏按钮上，则通知 CaptionButtons，否则将消息传递给主窗口
		switch (wParam) {
		case HTTOP:
		case HTTOPLEFT:
		case HTTOPRIGHT:
		case HTCAPTION:
		{
			// 将这些消息传给主窗口才能移动窗口或者调整窗口大小
			return _MessageHandler(msg, wParam, lParam);
		}
		case HTMINBUTTON:
		case HTMAXBUTTON:
		case HTCLOSE:
			Content()->TitleBar().CaptionButtons().PressButton((CaptionButton)wParam);
			// 在标题栏按钮上按下左键后我们便捕获光标，这样才能在释放时得到通知。注意捕获光标后
			// 便不会再收到 NC 族消息，这就是为什么我们要处理 WM_MOUSEMOVE 和 WM_LBUTTONUP
			SetCapture(_hwndTitleBar);
			break;
		}
		return 0;
	}
	// 在捕获光标时会收到
	case WM_LBUTTONUP:
	{
		ReleaseCapture();

		POINT cursorPos{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
		ClientToScreen(_hwndTitleBar, &cursorPos);
		wParam = _TitleBarMessageHandler(WM_NCHITTEST, 0, MAKELPARAM(cursorPos.x, cursorPos.y));
		[[fallthrough]];
	}
	case WM_NCLBUTTONUP:
	{
		// 处理鼠标在标题栏上释放。如果在标题栏按钮上，则通知 CaptionButtons，否则将消息传递给主窗口
		switch (wParam) {
		case HTTOP:
		case HTTOPLEFT:
		case HTTOPRIGHT:
		case HTCAPTION:
		{
			// 在可拖拽区域或上边框释放左键，将此消息传递给主窗口
			Content()->TitleBar().CaptionButtons().ReleaseButtons();
			return _MessageHandler(msg, wParam, lParam);
		}
		case HTMINBUTTON:
		case HTMAXBUTTON:
		case HTCLOSE:
			// 在标题栏按钮上释放左键
			Content()->TitleBar().CaptionButtons().ReleaseButton((CaptionButton)wParam);
			break;
		default:
			Content()->TitleBar().CaptionButtons().ReleaseButtons();
		}

		return 0;
	}
	case WM_NCRBUTTONDOWN:
	case WM_NCRBUTTONDBLCLK:
	case WM_NCRBUTTONUP:
		// 不关心右键，将它们传递给主窗口
		return _MessageHandler(msg, wParam, lParam);
	}

	return DefWindowProc(_hwndTitleBar, msg, wParam, lParam);
}

void MainWindow::_ResizeTitleBarWindow() noexcept {
	if (!_IsCustomTitleBarEnabled() || !_hwndTitleBar) {
		return;
	}

	TitleBarControl& titleBar = Content()->TitleBar();

	// 获取标题栏的边框矩形
	Rect rect{ 0.0f, 0.0f, (float)titleBar.ActualWidth(), (float)titleBar.ActualHeight() };
	rect = titleBar.TransformToVisual(*Content()).TransformBounds(rect);

	const float dpiScale = _CurrentDpi() / float(USER_DEFAULT_SCREEN_DPI);
	const uint32_t topBorderThickness = _GetTopBorderThickness();

	// 将标题栏窗口置于 XAML Islands 窗口上方，覆盖上边框和标题栏控件
	RECT clientRect;
	GetClientRect(Handle(), &clientRect);
	SetWindowPos(
		_hwndTitleBar,
		HWND_TOP,
		0,
		0,
		clientRect.right,
		topBorderThickness + (int)std::floorf(rect.Height * dpiScale + 1),	// 不知为何，直接向上取整有时无法遮盖 TitleBarControl
		SWP_SHOWWINDOW
	);

	if (_hwndMaximizeButton) {
		static const float captionButtonHeightInDips = [&]() {
			return titleBar.CaptionButtons().CaptionButtonSize().Height;
		}();

		const int captionButtonHeightInPixels = (int)std::ceilf(captionButtonHeightInDips * dpiScale);

		// 确保原生按钮和标题栏按钮高度相同
		MoveWindow(_hwndMaximizeButton, 0, topBorderThickness, clientRect.right, captionButtonHeightInPixels, FALSE);
	}

	// 设置标题栏窗口的最大化样式，这样才能展示正确的文字提示
	LONG_PTR style = GetWindowLongPtr(_hwndTitleBar, GWL_STYLE);
	SetWindowLongPtr(_hwndTitleBar, GWL_STYLE,
		_IsMaximized() ? style | WS_MAXIMIZE : style & ~WS_MAXIMIZE);
}

}
