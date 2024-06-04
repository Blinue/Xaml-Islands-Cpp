#include "pch.h"
#include "XamlApp.h"
#include "ThemeHelper.h"

namespace winrt {
using namespace XamlIslandsCpp::App;
}

namespace XamlIslandsCpp {

// 提前加载 twinapi.appcore.dll 和 threadpoolwinrt.dll 以避免退出时崩溃。应在 Windows.UI.Xaml.dll 被加载前调用
// 来自 https://github.com/CommunityToolkit/Microsoft.Toolkit.Win32/blob/6fb2c3e00803ea563af20f6bc9363091b685d81f/Microsoft.Toolkit.Win32.UI.XamlApplication/XamlApplication.cpp#L140
// 参见：https://github.com/microsoft/microsoft-ui-xaml/issues/7260#issuecomment-1231314776
static void FixThreadPoolCrash() noexcept {
	LoadLibraryEx(L"twinapi.appcore.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	LoadLibraryEx(L"threadpoolwinrt.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
}

bool XamlApp::Initialize(HINSTANCE hInstance) {
	_hInstance = hInstance;

	FixThreadPoolCrash();

	ThemeHelper::Initialize();

	// 初始化 UWP 应用
	_uwpApp = winrt::App();

	_mainWindow.Destroyed({ this, &XamlApp::_MainWindow_Destoryed });

	if (!_CreateMainWindow()) {
		return false;
	}

	return true;
}

int XamlApp::Run() {
	return _mainWindow.MessageLoop();
}

bool XamlApp::_CreateMainWindow(const WINDOWPLACEMENT* wp) noexcept {
	winrt::Settings settings = _uwpApp.Settings();

	if (!_mainWindow.Create(
		_hInstance,
		settings.Theme(),
		settings.Backdrop(),
		settings.IsCustomTitleBarEnabled(),
		wp
	)) {
		return false;
	}

	_uwpApp.HwndMain((uint64_t)_mainWindow.Handle());

	_themeChangedRevoker = settings.ThemeChanged(
		winrt::auto_revoke,
		[&](winrt::IInspectable const& sender, winrt::AppTheme theme) {
			winrt::Settings settings = sender.as<winrt::Settings>();
			_mainWindow.SetTheme(theme, settings.Backdrop());
		}
	);

	_backdropChangedRevoker = settings.BackdropChanged(
		winrt::auto_revoke,
		[this](winrt::IInspectable const& sender, winrt::WindowBackdrop backdrop) {
			winrt::Settings settings = sender.as<winrt::Settings>();
			if (_mainWindow.SetTheme(settings.Theme(), backdrop)) {
				// 由于无法更改 WS_EX_NOREDIRECTIONBITMAP 样式，必须重新创建主窗口
				shouldQuit = false;

				WINDOWPLACEMENT wp{ .length = sizeof(wp) };
				GetWindowPlacement(_mainWindow.Handle(), &wp);

				// 禁用关闭窗口的动画
				BOOL value = TRUE;
				DwmSetWindowAttribute(
					_mainWindow.Handle(), DWMWA_TRANSITIONS_FORCEDISABLED, &value, sizeof(value));

				winrt::CoreDispatcher dispatcher = _mainWindow.Content().Dispatcher();
				_mainWindow.Destroy();
				dispatcher.TryRunAsync(winrt::CoreDispatcherPriority::Normal, [this, wp]() {
					shouldQuit = true;
					_CreateMainWindow(&wp);
				});
			}
		}
	);

	_isCustomTitleBarEnabledChangedRevoker = settings.IsCustomTitleBarEnabledChanged(
		winrt::auto_revoke,
		[&](winrt::IInspectable const&, bool enabled) {
			_mainWindow.SetCustomTitleBar(enabled);
		}
	);

	return true;
}

void XamlApp::_MainWindow_Destoryed() {
	_themeChangedRevoker.revoke();
	_isCustomTitleBarEnabledChangedRevoker.revoke();
	_backdropChangedRevoker.revoke();

	_uwpApp.HwndMain(0);

	if (shouldQuit) {
		PostQuitMessage(0);
	}
}

}
