#include "pch.h"
#include "App.h"
#if __has_include("App.g.cpp")
#include "App.g.cpp"
#endif
#include <CoreWindow.h>
#include "Win32Helper.h"
#include "MainWindow.h"
#include "ThemeHelper.h"

using namespace ::XamlIslandsCpp;
using namespace winrt;
using namespace Windows::UI::ViewManagement;

namespace winrt::XamlIslandsCpp::implementation {

// 提前加载 twinapi.appcore.dll 和 threadpoolwinrt.dll 以避免退出时崩溃。应在 Windows.UI.Xaml.dll
// 被加载前调用，注意避免初始化全局变量时意外加载这个 dll，尤其是为了注册 DependencyProperty。
// 来自 https://github.com/CommunityToolkit/Microsoft.Toolkit.Win32/blob/6fb2c3e00803ea563af20f6bc9363091b685d81f/Microsoft.Toolkit.Win32.UI.XamlApplication/XamlApplication.cpp#L140
// 参见 https://github.com/microsoft/microsoft-ui-xaml/issues/7260#issuecomment-1231314776
static void FixThreadPoolCrash() noexcept {
	assert(!GetModuleHandle(L"Windows.UI.Xaml.dll"));
	LoadLibraryEx(L"twinapi.appcore.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	LoadLibraryEx(L"threadpoolwinrt.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
}

App& App::Get() {
	static com_ptr<App> instance = [] {
		FixThreadPoolCrash();
		return make_self<App>();
	}();

	return *instance;
}

App::App() {
#ifdef _DEBUG
	UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e) {
		if (IsDebuggerPresent()) {
			hstring errorMessage = e.Message();
			__debugbreak();
		}
	});
#endif
}

void App::Initialize() {
	_mainWindow = std::make_unique<class MainWindow>();

	// 初始化 XAML 框架。退出时也不要关闭，如果正在播放动画会崩溃。文档中的清空消息队列的做法无用。
	_windowsXamlManager = Hosting::WindowsXamlManager::InitializeForCurrentThread();

	if (CoreWindow coreWindow = CoreWindow::GetForCurrentThread()) {
		// Win10 中隐藏 DesktopWindowXamlSource 窗口
		if (!Win32Helper::GetOSVersion().IsWin11()) {
			HWND hwndDWXS;
			coreWindow.as<ICoreWindowInterop>()->get_WindowHandle(&hwndDWXS);
			ShowWindow(hwndDWXS, SW_HIDE);
		}

		_dispatcher = coreWindow.Dispatcher();
	}

	// 设置显示语言，不支持在运行时更改
	// ResourceContext::SetGlobalQualifierValue(L"Language", L"en-us");

	ThemeHelper::Initialize();

	_themeChangedRevoker = AppSettings::Get().ThemeChanged(
		auto_revoke, std::bind_front(&App::_AppSettings_ThemeChanged, this));
	_AppSettings_ThemeChanged(AppSettings::Get().Theme());

	if (!_mainWindow->Create()) {
		Quit();
	}
}

int App::Run() {
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		_mainWindow->HandleMessage(msg);
	}

	_colorValuesChangedRevoker.revoke();
	_themeChangedRevoker.Revoke();

	// 确保退出时所有事件回调都已撤销，既保持整洁又能防止析构全局变量时崩溃
	assert(_DEBUG_DELEGATE_COUNT == 0);

	return (int)msg.wParam;
}

void App::Quit() {
	_mainWindow->Destroy();
	PostQuitMessage(0);
}

const com_ptr<RootPage>& App::RootPage() const noexcept {
	assert(_mainWindow && *_mainWindow);
	return _mainWindow->Content();
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
				_dispatcher.RunAsync(CoreDispatcherPriority::Normal, [this] {
					_UpdateTheme();
				});
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
