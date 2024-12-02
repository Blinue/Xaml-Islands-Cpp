#include "pch.h"
#include "App.h"
#if __has_include("App.g.cpp")
#include "App.g.cpp"
#endif
#if __has_include("MyXamlMetaDataProvider.g.cpp")
#include "MyXamlMetaDataProvider.g.cpp"
#endif
#include <CoreWindow.h>
#include "Win32Helper.h"

using namespace XamlIslandsCpp;

namespace winrt::XamlIslandsCpp::implementation {

App::App() {
	// 初始化 XAML 框架
	_windowsXamlManager = Hosting::WindowsXamlManager::InitializeForCurrentThread();

	if (!Win32Helper::GetOSVersion().IsWin11()) {
		// Win10 中隐藏 DesktopWindowXamlSource 窗口
		if (CoreWindow coreWindow = CoreWindow::GetForCurrentThread()) {
			HWND hwndDWXS;
			coreWindow.as<ICoreWindowInterop>()->get_WindowHandle(&hwndDWXS);
			ShowWindow(hwndDWXS, SW_HIDE);
		}
	}

#ifdef _DEBUG
	UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e) {
		if (IsDebuggerPresent()) {
			hstring errorMessage = e.Message();
			__debugbreak();
		}
	});
#endif
}

App::~App() {
	Close();
}

void App::Close() {
	if (_isClosed) {
		return;
	}
	_isClosed = true;

	_windowsXamlManager.Close();
	_windowsXamlManager = nullptr;

	Exit();
}

}
