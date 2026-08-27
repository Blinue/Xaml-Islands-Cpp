#pragma once
#include "AppSettings.h"
#include "BorderlessWindow.h"
#include "RootPage.h"
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>

namespace XamlIslandsCpp {

class MainWindow : public BorderlessWindow<MainWindow> {
	using base_type = BorderlessWindow<MainWindow>;
	friend base_type;
	friend BaseWindow<MainWindow>;

public:
	~MainWindow() noexcept;

	bool Create(const WINDOWPLACEMENT* wp = nullptr) noexcept;

	void HandleMessage(const MSG& msg);

private:
	LRESULT _MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	bool _ShouldDrawBackground() const noexcept;

	void _DrawBackground(HDC hdc, const RECT& bkgRect) const noexcept;

	static LRESULT CALLBACK _TitleBarWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	LRESULT _TitleBarMessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	void _ResizeTitleBarWindow() noexcept;

	void _UpdateTheme() noexcept;

	void _UpdateBackdrop() noexcept;

	// 支持在创建窗口前调用
	winrt::fire_and_forget _UpdateCustomTitleBar() noexcept;

	winrt::com_ptr<winrt::XamlIslandsCpp::implementation::RootPage> _rootPage;

	HWND _hwndXamlIsland = NULL;
	winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource _xamlSource{ nullptr };
	winrt::com_ptr<IDesktopWindowXamlSourceNative2> _xamlSourceNative2;

	HWND _hwndTitleBar = NULL;
	HWND _hwndMaximizeButton = NULL;

	Event<bool>::EventRevoker _appThemeChangedRevoker;
	Event<WindowBackdrop>::EventRevoker _backdropChangedRevoker;
	Event<bool>::EventRevoker _isCustomTitleBarEnabledChangedRevoker;

	HBRUSH _hbrBackground = NULL;

	bool _isTrackingMouse = false;
	// 防止重新创建主窗口时退出
	bool _isClosingForRecreate = false;
	bool _isSmoothResizeEnabled = false;
};

}
