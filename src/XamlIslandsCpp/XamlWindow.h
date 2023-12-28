#pragma once
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>
#include <CoreWindow.h>

template <typename T, typename C>
class XamlWindowT {
public:
	virtual ~XamlWindowT() {
		DestroyWindow(_hWnd);
	}

	int MessageLoop() {
		MSG msg{};
		while (GetMessage(&msg, nullptr, 0, 0)) {
			// XAML Islands 会吞掉 Alt+F4，需要特殊处理
			// https://github.com/microsoft/microsoft-ui-xaml/issues/2408
			if (msg.message == WM_SYSKEYDOWN && msg.wParam == VK_F4) [[unlikely]] {
				SendMessage(GetAncestor(msg.hwnd, GA_ROOT), msg.message, msg.wParam, msg.lParam);
				continue;
			}

			BOOL processed = FALSE;
			HRESULT hr = _xamlSourceNative2->PreTranslateMessage(&msg, &processed);
			if (SUCCEEDED(hr) && processed) {
				continue;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return (int)msg.wParam;
	}

	HWND Handle() const noexcept {
		return _hWnd;
	}

protected:
	using base_type = XamlWindowT<T, C>;

	static LRESULT CALLBACK _WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
		if (msg == WM_NCCREATE) {
			XamlWindowT* that = (XamlWindowT*)(((CREATESTRUCT*)lParam)->lpCreateParams);
			assert(that && !that->_hWnd);
			that->_hWnd = hWnd;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)that);
		} else if (T* that = (T*)GetWindowLongPtr(hWnd, GWLP_USERDATA)) {
			return that->_MessageHandler(msg, wParam, lParam);
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	void _SetContent(C const& content) {
		_content = content;
		// Xaml 控件加载完成后显示主窗口
		_content.Loaded([this](winrt::IInspectable const&, winrt::RoutedEventArgs const&) -> winrt::IAsyncAction {
			co_await _content.Dispatcher().RunAsync(winrt::CoreDispatcherPriority::Normal, [hWnd(_hWnd)]() {
				// 防止窗口显示时背景闪烁
				// https://stackoverflow.com/questions/69715610/how-to-initialize-the-background-color-of-win32-app-to-something-other-than-whit
				SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				ShowWindow(hWnd, SW_SHOWNORMAL);
			});
		});

		// 初始化 XAML Islands
		_xamlSource = winrt::DesktopWindowXamlSource();
		_xamlSourceNative2 = _xamlSource.as<IDesktopWindowXamlSourceNative2>();

		auto interop = _xamlSource.as<IDesktopWindowXamlSourceNative>();
		interop->AttachToWindow(_hWnd);
		interop->get_WindowHandle(&_hwndXamlIsland);
		_xamlSource.Content(content);

		// 焦点始终位于 _hwndXamlIsland 中
		_xamlSource.TakeFocusRequested(
			[](winrt::DesktopWindowXamlSource const& sender,
			winrt::DesktopWindowXamlSourceTakeFocusRequestedEventArgs const& args
		) {
			winrt::XamlSourceFocusNavigationReason reason = args.Request().Reason();
			if (reason < winrt::XamlSourceFocusNavigationReason::Left) {
				sender.NavigateFocus(args.Request());
			}
		});

		// 防止第一次收到 WM_SIZE 消息时 RootPage 尺寸为 0
		_OnResize();
	}

	LRESULT _MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
		switch (msg) {
		case WM_SHOWWINDOW:
		{
			if (wParam == TRUE) {
				// 将焦点置于 XAML Islands 窗口可以修复按 Alt 键会导致 UI 无法交互的问题
				SetFocus(_hwndXamlIsland);
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
		case WM_DPICHANGED:
		{
			RECT* newRect = (RECT*)lParam;
			SetWindowPos(_hWnd,
				NULL,
				newRect->left,
				newRect->top,
				newRect->right - newRect->left,
				newRect->bottom - newRect->top,
				SWP_NOZORDER | SWP_NOACTIVATE
			);

			break;
		}
		case WM_MOVING:
		{
			if (_hwndXamlIsland) {
				_RepositionXamlPopups(_content.XamlRoot(), false);
			}

			return 0;
		}
		case WM_MENUCHAR:
		{
			// 防止按 Alt+Key 时发出铃声
			return MAKELRESULT(0, MNC_CLOSE);
		}
		case WM_SYSCOMMAND:
		{
			// 禁用按 Alt 键会激活菜单的行为，它使用户界面无法交互
			if ((wParam & 0xFFF0) == SC_KEYMENU && lParam == 0) {
				return 0;
			}

			break;
		}
		case WM_SIZE:
		{
			if (wParam != SIZE_MINIMIZED) {
				_OnResize();

				if (_hwndXamlIsland) {
					// 使 ContentDialog 跟随窗口尺寸调整
					// 来自 https://github.com/microsoft/microsoft-ui-xaml/issues/3577#issuecomment-1399250405
					if (winrt::CoreWindow coreWindow = winrt::CoreWindow::GetForCurrentThread()) {
						HWND hwndDWXS;
						coreWindow.as<ICoreWindowInterop>()->get_WindowHandle(&hwndDWXS);
						PostMessage(hwndDWXS, WM_SIZE, wParam, lParam);
					}

					[](C const& content)->winrt::fire_and_forget {
						co_await content.Dispatcher().RunAsync(winrt::CoreDispatcherPriority::Normal, [xamlRoot(content.XamlRoot())]() {
							_RepositionXamlPopups(xamlRoot, true);
						});
					}(_content);
				}
			}

			return 0;
		}
		}

		return DefWindowProc(_hWnd, msg, wParam, lParam);
	}

	HWND _hWnd = NULL;
	C _content{ nullptr };
	
private:
	void _OnResize() noexcept {
		RECT clientRect;
		GetClientRect(_hWnd, &clientRect);
		SetWindowPos(_hwndXamlIsland, NULL, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, SWP_SHOWWINDOW | SWP_NOACTIVATE);
	}

	static void _RepositionXamlPopups(winrt::XamlRoot const& root, bool closeFlyoutPresenter) {
		if (!root) {
			return;
		}

		for (const auto& popup : winrt::VisualTreeHelper::GetOpenPopupsForXamlRoot(root)) {
			if (closeFlyoutPresenter) {
				auto className = winrt::get_class_name(popup.Child());
				if (className == winrt::name_of<winrt::Controls::FlyoutPresenter>() ||
					className == winrt::name_of<winrt::Controls::MenuFlyoutPresenter>()
					) {
					popup.IsOpen(false);
					continue;
				}
			}

			// 取自 https://github.com/CommunityToolkit/Microsoft.Toolkit.Win32/blob/229fa3cd245ff002906b2a594196b88aded25774/Microsoft.Toolkit.Forms.UI.XamlHost/WindowsXamlHostBase.cs#L180

			// Toggle the CompositeMode property, which will force all windowed Popups
			// to reposition themselves relative to the new position of the host window.
			auto compositeMode = popup.CompositeMode();

			// Set CompositeMode to some value it currently isn't set to.
			if (compositeMode == winrt::ElementCompositeMode::SourceOver) {
				popup.CompositeMode(winrt::ElementCompositeMode::MinBlend);
			} else {
				popup.CompositeMode(winrt::ElementCompositeMode::SourceOver);
			}

			// Restore CompositeMode to whatever it was originally set to.
			popup.CompositeMode(compositeMode);
		}
	}

	HWND _hwndXamlIsland = NULL;
	winrt::DesktopWindowXamlSource _xamlSource{ nullptr };
	winrt::com_ptr<IDesktopWindowXamlSourceNative2> _xamlSourceNative2;
};
