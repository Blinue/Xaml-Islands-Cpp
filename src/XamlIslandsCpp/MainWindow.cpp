#include "pch.h"
#include "App.h"
#include "CaptionButtonsControl.h"
#include "CommonSharedConstants.h"
#include "MainWindow.h"
#include "SmoothResizeHelper.h"
#include "ThemeHelper.h"
#include "TitleBarControl.h"
#include "Win32Helper.h"
#include "XamlHelper.h"
#include <CoreWindow.h>
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>

using namespace winrt::XamlIslandsCpp::implementation;
namespace winrt {
using namespace Windows::UI::Xaml::Hosting;
}

// 来自 https://learn.microsoft.com/en-us/windows/apps/api-reference/interface-members/ixamlsourcetransparency-isbackgroundtransparent
DECLARE_INTERFACE_IID_(IXamlSourceTransparency, ::IInspectable, "06636C29-5A17-458D-8EA2-2422D997A922") {
	STDMETHOD(get_IsBackgroundTransparent)(boolean* value) PURE;
	STDMETHOD(put_IsBackgroundTransparent)(boolean value) PURE;
};

namespace XamlIslandsCpp {

MainWindow::~MainWindow() noexcept {
	Destroy();
}

bool MainWindow::Create(const WINDOWPLACEMENT* wp) noexcept {
	[[maybe_unused]] static const int _ = []() {
		const HINSTANCE hInstance = Win32Helper::GetModuleInstanceHandle();

		WNDCLASSEXW wcex = {
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

	AppSettings& settings = AppSettings::Get();

	// 创建窗口前设置能避免不必要的开销
	_SetCustomTitleBar(settings.IsCustomTitleBarEnabled());

	const HINSTANCE hInstance = Win32Helper::GetModuleInstanceHandle();
	CreateWindowEx(
		(Win32Helper::GetOSVersion().Is22H2OrNewer() &&
			settings.Backdrop() != WindowBackdrop::SolidColor) ? WS_EX_NOREDIRECTIONBITMAP : 0,
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

	// 首次设置窗口主题应强制更新
	_SetTheme(App::Get().IsLightTheme(), true);
	// 无视返回值，因为 WS_EX_NOREDIRECTIONBITMAP 样式是正确的
	_SetBackdrop(settings.Backdrop(), true);

	_rootPage = winrt::make_self<RootPage>();

	// 初始化 XAML Islands
	_xamlSource = winrt::DesktopWindowXamlSource();
	_xamlSourceNative2 = _xamlSource.as<IDesktopWindowXamlSourceNative2>();

	_xamlSourceNative2->AttachToWindow(Handle());
	_xamlSourceNative2->get_WindowHandle(&_hwndXamlIsland);
	_xamlSource.Content(*_rootPage);

	// 焦点始终位于 _hwndXamlIsland 中
	_xamlSource.TakeFocusRequested(
		[](winrt::DesktopWindowXamlSource const& sender,
		winrt::DesktopWindowXamlSourceTakeFocusRequestedEventArgs const& args
	) {
		sender.NavigateFocus(args.Request());
	});

	// XAML Islands 默认存在背景色，下面的调用使该背景透明，从而显露出 DWM 绘制的背景。虽然从
	// Win11 22H2 开始 DWM 才开始支持绘制 Mica 等背景，但 XAML Islands 的背景色本来就不符合
	// 直觉，去掉没坏处。
	if (auto xst = winrt::Window::Current().try_as<IXamlSourceTransparency>()) {
		xst->put_IsBackgroundTransparent(true);
	}

	_isSmoothResizeEnabled = SmoothResizeHelper::EnableResizeSync(Handle(), App::Get());

	// 隐藏原生标题栏上的图标
	SetWindowThemeNonClientAttributes(Handle(), WTNCA_NODRAWICON | WTNCA_NOSYSMENU, WTNCA_VALIDBITS);

	// 1. 刷新窗口边框
	// 2. 防止窗口显示时背景闪烁: https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit
	SetWindowPos(Handle(), NULL, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOCOPYBITS);

	// Xaml 控件加载完成后显示主窗口
	if (wp) {
		_rootPage->Loaded([this, wp(*wp)](winrt::IInspectable const&, winrt::RoutedEventArgs const&) {
			// 禁用显示窗口的动画
			BOOL value = TRUE;
			DwmSetWindowAttribute(Handle(), DWMWA_TRANSITIONS_FORCEDISABLED, &value, sizeof(value));

			SetWindowPlacement(Handle(), &wp);

			value = FALSE;
			DwmSetWindowAttribute(Handle(), DWMWA_TRANSITIONS_FORCEDISABLED, &value, sizeof(value));
		});
	} else {
		_rootPage->Loaded([this](winrt::IInspectable const&, winrt::RoutedEventArgs const&) {
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

	_rootPage->TitleBar().SizeChanged([this](winrt::IInspectable const&, winrt::SizeChangedEventArgs const&) {
		_ResizeTitleBarWindow();
	});

	_appThemeChangedRevoker = App::Get().ThemeChanged(
		winrt::auto_revoke, [this](bool isLightTheme) { _SetTheme(isLightTheme); });

	_backdropChangedRevoker = settings.BackdropChanged(
		winrt::auto_revoke,
		[this](WindowBackdrop backdrop) {
			if (!_SetBackdrop(backdrop)) {
				return;
			}

			// 由于无法更改 WS_EX_NOREDIRECTIONBITMAP 样式，必须重新创建主窗口。
			// 应延迟执行，让 UI 完成更新。
			App::Get().Dispatcher().TryEnqueue([this]() {
				WINDOWPLACEMENT wp{ .length = sizeof(wp) };
				GetWindowPlacement(Handle(), &wp);

				// 禁用关闭窗口的动画
				BOOL value = TRUE;
				DwmSetWindowAttribute(Handle(), DWMWA_TRANSITIONS_FORCEDISABLED, &value, sizeof(value));

				// 重新构造 MainWindow
				_isClosingForRecreate = true;
				std::destroy_at(this);
				std::construct_at(this);

				Create(&wp);
			});
		}
	);

	_isCustomTitleBarEnabledChangedRevoker = settings.IsCustomTitleBarEnabledChanged(
		winrt::auto_revoke,
		[&](bool enabled) {
			if (_IsBorderless() == enabled) {
				return;
			}

			ShowWindow(_hwndTitleBar, enabled ? SW_SHOW : SW_HIDE);

			if (enabled) {
				_SetCustomTitleBar(true);
			} else {
				// 优化动画
				App::Get().Dispatcher().TryEnqueue([this]() -> winrt::fire_and_forget {
					MainWindow* that = this;
					winrt::DispatcherQueue dispatcher = App::Get().Dispatcher();
					co_await 10ms;
					co_await dispatcher;
					that->_SetCustomTitleBar(false);
				});
			}
		}
	);

	return true;
}

void MainWindow::HandleMessage(const MSG& msg) {
	// XAML Islands 会吞掉 Alt+F4，需要特殊处理
	// https://github.com/microsoft/microsoft-ui-xaml/issues/2408
	if (msg.message == WM_SYSKEYDOWN && msg.wParam == VK_F4) [[unlikely]] {
		SendMessage(GetAncestor(msg.hwnd, GA_ROOT), msg.message, msg.wParam, msg.lParam);
		return;
	}

	if (_xamlSourceNative2) {
		BOOL processed = FALSE;
		HRESULT hr = _xamlSourceNative2->PreTranslateMessage(&msg, &processed);
		if (SUCCEEDED(hr) && processed) {
			return;
		}
	}

	TranslateMessage(&msg);
	DispatchMessage(&msg);
}

LRESULT MainWindow::_MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	switch (msg) {
	case WM_SIZE:
	{
		base_type::_MessageHandler(msg, wParam, lParam);

		if (wParam != SIZE_MINIMIZED && _rootPage) {
			if (_isSmoothResizeEnabled) {
				SmoothResizeHelper::SyncWindowSize(Handle(), App::Get());
			}

			// 调整 XAML Islands 窗口尺寸
			{
				int clientWidth = LOWORD(lParam);
				int clientHeight = HIWORD(lParam);
				// XAML Islands 窗口在上边框下方。Win10 和 Win11 中上边框都在客户区内。
				int topBorderThickness = (int)_GetTopBorderThickness();

				// SWP_NOZORDER 确保 XAML Islands 窗口始终在标题栏窗口下方，否则主窗口在调整大小时会闪烁
				SetWindowPos(
					_hwndXamlIsland,
					NULL,
					0,
					topBorderThickness,
					clientWidth,
					clientHeight - topBorderThickness,
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW
				);
			}

			if (_IsBorderless()) {
				_ResizeTitleBarWindow();
				_rootPage->TitleBar().CaptionButtons().IsWindowMaximized(_IsMaximized());
			}

			// 使 ContentDialog 跟随窗口尺寸调整，来自
			// https://github.com/microsoft/microsoft-ui-xaml/issues/3577#issuecomment-1399250405
			if (winrt::CoreWindow coreWindow = winrt::CoreWindow::GetForCurrentThread()) {
				HWND hwndDWXS;
				coreWindow.as<ICoreWindowInterop>()->get_WindowHandle(&hwndDWXS);
				SendMessage(hwndDWXS, WM_SIZE, wParam, lParam);
			}

			App::Get().Dispatcher().TryEnqueue([xamlRoot(_rootPage->XamlRoot())]() {
				XamlHelper::RepositionXamlPopups(xamlRoot, true);
			});
		}

		return 0;
	}
	case WM_MOVING:
	{
		if (_hwndXamlIsland) {
			XamlHelper::RepositionXamlPopups(_rootPage->XamlRoot(), false);
		}

		return 0;
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
		if (_IsBorderless() && wParam == HTCAPTION) {
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
				MENUITEMINFO mii = {
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
		if (_rootPage) {
			if (_IsBorderless()) {
				_rootPage->TitleBar().IsWindowActive(LOWORD(wParam) != WA_INACTIVE);
			}

			if (LOWORD(wParam) == WA_INACTIVE) {
				XamlHelper::CloseComboBoxPopup(_rootPage->XamlRoot());
			}
		}
		
		break;
	}
	case WM_GETTITLEBARINFOEX:
	{
		if (_IsBorderless()) {
			// 为了支持 Win11 的贴靠布局，需要返回最大化按钮的边界矩形
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
		// 为了和第三方程序兼容，应确保主窗口本身可以正确响应 WM_NCHITTEST。
		// 见 https://github.com/microsoft/terminal/issues/8795

		if (!_IsBorderless()) {
			break;
		}

		// 基类处理非客户区
		LRESULT ht = base_type::_MessageHandler(msg, wParam, lParam);
		if (ht != HTCLIENT || !_hwndTitleBar) {
			return ht;
		}

		const POINT cursorPos{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };

		RECT clientRect;
		Win32Helper::GetClientScreenRect(Handle(), clientRect);

		// _hwndTitleBar 为标题栏区域，下方是客户区
		RECT titlebarWndRect{};
		GetWindowRect(_hwndTitleBar, &titlebarWndRect);
		if (!PtInRect(&titlebarWndRect, cursorPos)) {
			return HTCLIENT;
		}

		static const winrt::Size buttonSizeInDips = [this]() {
			return _rootPage->TitleBar().CaptionButtons().CaptionButtonSize();
		}();

		float buttonWidthInPixels = buttonSizeInDips.Width * _CurrentDpi() / USER_DEFAULT_SCREEN_DPI;
		float buttonHeightInPixels = buttonSizeInDips.Height * _CurrentDpi() / USER_DEFAULT_SCREEN_DPI;

		if (cursorPos.y >= clientRect.top + _GetTopBorderThickness() + buttonHeightInPixels) {
			// 光标位于标题按钮下方，如果标题栏很宽，这里也可以拖动
			return HTCAPTION;
		}

		// 从右向左检查光标是否位于某个标题栏按钮上
		LONG cursorToRight = clientRect.right - cursorPos.x;
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
	case WM_SYSCOMMAND:
	{
		// 根据文档，wParam 的低四位供系统内部使用
		switch (wParam & 0xFFF0) {
		case SC_MINIMIZE:
		{
			// 最小化前关闭 ComboBox。不能在 WM_SIZE 中处理，该消息发送于最小化之后，会导致 ComboBox 无法交互
			if (_rootPage) {
				XamlHelper::CloseComboBoxPopup(_rootPage->XamlRoot());
			}
			break;
		}
		case SC_KEYMENU:
		{
			// 禁用按 Alt 键会激活窗口菜单的行为，它使用户界面无法交互
			if (lParam == 0) {
				return 0;
			}
			break;
		}
		}

		break;
	}
	case WM_DESTROY:
	{
		_appThemeChangedRevoker.Revoke();
		_isCustomTitleBarEnabledChangedRevoker.Revoke();
		_backdropChangedRevoker.Revoke();
		
		// 标题栏窗口经常使用 _rootPage，确保在关闭 DWXS 前销毁
		DestroyWindow(_hwndTitleBar);

		// 确保关闭过程中 _rootPage 已经为空
		_rootPage = nullptr;

		_xamlSourceNative2 = nullptr;
		// 必须手动重置 Content，否则会内存泄露，使 RootPage 无法析构
		_xamlSource.Content(nullptr);
		_xamlSource.Close();
		_xamlSource = nullptr;

		// 关闭 DesktopWindowXamlSource 后应清空消息队列以确保 RootPage 析构
		MSG msg1;
		while (PeekMessage(&msg1, nullptr, 0, 0, PM_REMOVE)) {
			DispatchMessage(&msg1);
		}
		// 偶尔清空消息队列无用，需要再清空一次，不确定是否 100% 可靠。谢谢你，XAML Islands！
		Sleep(0);
		while (PeekMessage(&msg1, nullptr, 0, 0, PM_REMOVE)) {
			DispatchMessage(&msg1);
		}
		
		if (!_isClosingForRecreate) {
			PostQuitMessage(0);
		}

		break;
	}
	}

	return BorderlessWindowT::_MessageHandler(msg, wParam, lParam);
}

void MainWindow::_DrawBackground(HDC hdc, const RECT& bkgRect) const noexcept {
	static bool isLightBrush = _isLightTheme;
	static HBRUSH backgroundBrush = CreateSolidBrush(isLightBrush ?
		CommonSharedConstants::LIGHT_TINT_COLOR : CommonSharedConstants::DARK_TINT_COLOR);

	if (isLightBrush != _isLightTheme) {
		isLightBrush = _isLightTheme;
		DeleteBrush(backgroundBrush);
		backgroundBrush = CreateSolidBrush(isLightBrush ?
			CommonSharedConstants::LIGHT_TINT_COLOR : CommonSharedConstants::DARK_TINT_COLOR);
	}

	// 绘制深色背景时需要注意调使用 DwmExtendFrameIntoClientArea 后深色背景会被视为透明。解决方案来自
	// https://github.com/microsoft/terminal/blob/0ee2c74cd432eda153f3f3e77588164cde95044f/src/cascadia/WindowsTerminal/NonClientIslandWindow.cpp#L1030-L1047
	if (!isLightBrush && Win32Helper::GetOSVersion().IsWin10()) {
		HDC opaqueDc;
		BP_PAINTPARAMS params = {
			.cbSize = sizeof(params),
			.dwFlags = BPPF_NOCLIP | BPPF_ERASE
		};
		HPAINTBUFFER buf = BeginBufferedPaint(hdc, &bkgRect, BPBF_TOPDOWNDIB, &params, &opaqueDc);
		if (buf && opaqueDc) {
			FillRect(opaqueDc, &bkgRect, backgroundBrush);
			BufferedPaintSetAlpha(buf, nullptr, 255);
			EndBufferedPaint(buf, TRUE);
		}
	} else {
		FillRect(hdc, &bkgRect, backgroundBrush);
	}
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
	case WM_NCHITTEST:
	{
		// 和主窗口一致
		return _MessageHandler(WM_NCHITTEST, wParam, lParam);
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
		CaptionButtonsControl& captionButtons = _rootPage->TitleBar().CaptionButtons();

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
			if (!_isTrackingMouse && msg == WM_NCMOUSEMOVE) {
				TRACKMOUSEEVENT ev = {
					.cbSize = sizeof(TRACKMOUSEEVENT),
					.dwFlags = TME_LEAVE | TME_NONCLIENT,
					.hwndTrack = _hwndTitleBar,
					.dwHoverTime = HOVER_DEFAULT // 不关心 HOVER 消息
				};
				TrackMouseEvent(&ev);
				_isTrackingMouse = true;
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
			_rootPage->TitleBar().CaptionButtons().LeaveButtons();
		} else {
			// 然后检查鼠标在标题栏上的位置
			LRESULT hit = _TitleBarMessageHandler(WM_NCHITTEST, 0, MAKELPARAM(cursorPos.x, cursorPos.y));
			if (hit != HTMINBUTTON && hit != HTMAXBUTTON && hit != HTCLOSE) {
				_rootPage->TitleBar().CaptionButtons().LeaveButtons();
			}
		}

		_isTrackingMouse = false;
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
			_rootPage->TitleBar().CaptionButtons().PressButton((CaptionButton)wParam);
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
			_rootPage->TitleBar().CaptionButtons().ReleaseButtons();
			return _MessageHandler(msg, wParam, lParam);
		}
		case HTMINBUTTON:
		case HTMAXBUTTON:
		case HTCLOSE:
			// 在标题栏按钮上释放左键
			_rootPage->TitleBar().CaptionButtons().ReleaseButton((CaptionButton)wParam);
			break;
		default:
			_rootPage->TitleBar().CaptionButtons().ReleaseButtons();
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
	if (!_IsBorderless() || !_hwndTitleBar) {
		return;
	}

	TitleBarControl& titleBar = _rootPage->TitleBar();

	// 获取标题栏的边框矩形
	winrt::Rect rect{ 0.0f, 0.0f, (float)titleBar.ActualWidth(), (float)titleBar.ActualHeight() };
	rect = titleBar.TransformToVisual(*_rootPage).TransformBounds(rect);

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

void MainWindow::_SetTheme(bool isLightTheme, bool force) noexcept {
	assert(Handle());

	if (std::exchange(_isLightTheme, isLightTheme) == isLightTheme && !force) {
		return;
	}

	// 在 Win10 中如果自定义标题栏，那么即使在亮色主题下也应使用暗色边框，这也是 UWP 窗口的行为
	ThemeHelper::SetWindowTheme(
		Handle(),
		Win32Helper::GetOSVersion().IsWin11() || !_IsBorderless() ? !isLightTheme : true,
		!isLightTheme
	);
}

bool MainWindow::_SetBackdrop(WindowBackdrop value, bool force) noexcept {
	assert(Handle());

	if (!Win32Helper::GetOSVersion().Is22H2OrNewer() || _backdrop == value) {
		return false;
	}

	if (!force) {
		if (_backdrop == value) {
			return false;
		}

		// 在纯色和其他背景间切换需要重新创建窗口，因为需要更改 WS_EX_NOREDIRECTIONBITMAP 样式
		bool wasSolidColor = _backdrop == WindowBackdrop::SolidColor;
		bool isSolidColor = value == WindowBackdrop::SolidColor;
		if (wasSolidColor != isSolidColor) {
			return true;
		}
	}

	_backdrop = value;

	static const DWM_SYSTEMBACKDROP_TYPE BACKDROP_MAP[] = {
		DWMSBT_AUTO, DWMSBT_TRANSIENTWINDOW, DWMSBT_MAINWINDOW, DWMSBT_TABBEDWINDOW
	};
	DWM_SYSTEMBACKDROP_TYPE attrValue = BACKDROP_MAP[(int)value];
	DwmSetWindowAttribute(Handle(), DWMWA_SYSTEMBACKDROP_TYPE, &attrValue, sizeof(attrValue));
	
	return false;
}

void MainWindow::_SetCustomTitleBar(bool enabled) noexcept {
	if (_IsBorderless() == enabled) {
		return;
	}

	_SetBorderless(enabled);

	// Win10 中需要更新边框主题
	if (Win32Helper::GetOSVersion().IsWin10() && Handle()) {
		_SetTheme(_isLightTheme, true);
	}
}

}
