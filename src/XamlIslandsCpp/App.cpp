#include "pch.h"
#include "App.h"
#if __has_include("App.g.cpp")
#include "App.g.cpp"
#endif
#include "MainWindow.h"
#include "ThemeHelper.h"
#include "Win32Helper.h"
#include <CoreWindow.h>

using namespace ::XamlIslandsCpp;
using namespace winrt;
using namespace Windows::UI::ViewManagement;

namespace winrt::XamlIslandsCpp::implementation {

App& App::Get() {
	static com_ptr<App> instance = make_self<App>();
	return *instance;
}

App::App() {
#ifdef _DEBUG
	UnhandledException([](IInspectable const&, UnhandledExceptionEventArgs const& e) {
		if (IsDebuggerPresent()) {
			hstring errorMessage = e.Message();
			__debugbreak();
		}
	});
#endif

	// !!! HACK !!!
	// 将 Application 泄露以避免退出时崩溃。XAML Islands 在关闭时存在大量 bug，而且不同的系统版
	// 本会在不同的地方崩溃，我们索性主动泄露来一劳永逸地解决问题。也可以通过调用 TerminateProcess
	// 来避免崩溃，不过这样全局变量和静态变量无法触发析构。
	AddRef();
}

bool App::Initialize() {
	_mainWindow = std::make_unique<class MainWindow>();

	// 初始化 XAML 框架。退出时也不要关闭，如果正在播放动画会崩溃。文档中的清空消息队列的做法无用。
	_windowsXamlManager = Hosting::WindowsXamlManager::InitializeForCurrentThread();

	// Win10 中 CoreDispatcher.RunAsync 存在内存泄露，因此我们始终使用 DispatcherQueue。
	// 初始化 WindowsXamlManager 时已经创建 DispatcherQueue。
	_dispatcher = winrt::DispatcherQueue::GetForCurrentThread();

	// Win10 中隐藏 DesktopWindowXamlSource 窗口
	if (Win32Helper::GetOSVersion().IsWin10()) {
		if (CoreWindow coreWindow = CoreWindow::GetForCurrentThread()) {
			HWND hwndDWXS;
			coreWindow.try_as<ICoreWindowInterop>()->get_WindowHandle(&hwndDWXS);
			ShowWindow(hwndDWXS, SW_HIDE);
		}
	}

	// 设置显示语言，不支持在运行时更改
	// ResourceContext::SetGlobalQualifierValue(L"Language", L"en-us");

	ThemeHelper::Initialize();

	_themeChangedRevoker = AppSettings::Get().ThemeChanged(
		auto_revoke, std::bind_front(&App::_AppSettings_ThemeChanged, this));
	_AppSettings_ThemeChanged(AppSettings::Get().Theme());

	if (!_mainWindow->Create()) {
		return false;
	}

	return true;
}

int App::Run() {
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		_mainWindow->HandleMessage(msg);
	}

	_colorValuesChangedRevoker.revoke();
	_themeChangedRevoker.Revoke();

	// 确保退出时所有事件回调都已撤销
	assert(_DEBUG_DELEGATE_COUNT == 0);

	return (int)msg.wParam;
}

void App::Quit() {
	_mainWindow.reset();
}

void App::_AppSettings_ThemeChanged(AppTheme) {
	_UpdateColorValuesChangedRevoker();
	_UpdateTheme();
}

void App::_UpdateColorValuesChangedRevoker() {
	if (AppSettings::Get().Theme() == AppTheme::System) {
		_colorValuesChangedRevoker = _uiSettings.ColorValuesChanged(
			auto_revoke,
			[this](const auto&, const auto&) {
				_dispatcher.TryEnqueue([this] { _UpdateTheme(); });
			}
		);
	} else {
		_colorValuesChangedRevoker.revoke();
	}
}

// 来自 https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/apply-windows-themes#know-when-dark-mode-is-enabled
static bool IsColorLight(const winrt::Windows::UI::Color& clr) noexcept {
	return 5 * clr.G + 2 * clr.R + clr.B > 8 * 128;
}

void App::_UpdateTheme() {
	AppTheme theme = AppSettings::Get().Theme();

	bool isLightTheme = false;
	if (theme == AppTheme::System) {
		// 前景色是亮色表示当前是深色主题
		isLightTheme = !IsColorLight(_uiSettings.GetColorValue(UIColorType::Foreground));
	} else {
		isLightTheme = theme == AppTheme::Light;
	}

	if (_isLightTheme != isLightTheme) {
		_isLightTheme = isLightTheme;
		ThemeChanged.Invoke(isLightTheme);
	}
}

}
