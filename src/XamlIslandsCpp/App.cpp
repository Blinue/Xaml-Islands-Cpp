#include "pch.h"
#include "App.h"
#if __has_include("App.g.cpp")
#include "App.g.cpp"
#endif
#include <CoreWindow.h>
#include "Win32Helper.h"

using namespace XamlIslandsCpp;

namespace winrt::XamlIslandsCpp::implementation {

App::App() {
#ifdef _DEBUG
	UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e) {
		if (IsDebuggerPresent()) {
			hstring errorMessage = e.Message();
			__debugbreak();
		}
	});
#endif

	// 初始化 XAML 框架。退出时也不要关闭，如果正在播放动画会崩溃。文档中的清空消息队列的做法无用。
	_windowsXamlManager = Hosting::WindowsXamlManager::InitializeForCurrentThread();

	if (!Win32Helper::GetOSVersion().IsWin11()) {
		// Win10 中隐藏 DesktopWindowXamlSource 窗口
		if (CoreWindow coreWindow = CoreWindow::GetForCurrentThread()) {
			HWND hwndDWXS;
			coreWindow.as<ICoreWindowInterop>()->get_WindowHandle(&hwndDWXS);
			ShowWindow(hwndDWXS, SW_HIDE);
		}
	}

	// 设置显示语言，不支持在运行时更改
	// ResourceContext::SetGlobalQualifierValue(L"Language", L"en-us");
}

}
