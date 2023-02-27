#include "pch.h"
#include "XamlApp.h"
#include "Utils.h"
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>

// https://github.com/microsoft/microsoft-ui-xaml/issues/7260#issuecomment-1231314776
// 提前加载 threadpoolwinrt.dll 以避免退出时崩溃。应在 Windows.UI.Xaml.dll 被加载前调用
static void FixThreadPoolCrash() noexcept {
	LoadLibraryEx(L"threadpoolwinrt.dll", nullptr, 0);
}

bool XamlApp::Initialize(HINSTANCE hInstance) {
	FixThreadPoolCrash();

	// 初始化 UWP 应用
	_uwpApp = winrt::XamlIslandsCpp::App::App();

	if (!_mainWindow.Initialize(hInstance)) {
		return false;
	}

	return true;
}

int XamlApp::Run() {
	return _mainWindow.MessageLoop();
}
