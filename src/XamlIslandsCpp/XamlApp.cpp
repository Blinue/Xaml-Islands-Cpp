#include "pch.h"
#include "XamlApp.h"
#include "ThemeHelper.h"

namespace XamlIslandsCpp {

// 提前加载 twinapi.appcore.dll 和 threadpoolwinrt.dll 以避免退出时崩溃。应在 Windows.UI.Xaml.dll 被加载前调用
// 来自 https://github.com/CommunityToolkit/Microsoft.Toolkit.Win32/blob/6fb2c3e00803ea563af20f6bc9363091b685d81f/Microsoft.Toolkit.Win32.UI.XamlApplication/XamlApplication.cpp#L140
// 参见：https://github.com/microsoft/microsoft-ui-xaml/issues/7260#issuecomment-1231314776
static void FixThreadPoolCrash() noexcept {
	LoadLibraryEx(L"twinapi.appcore.dll", nullptr, 0);
	LoadLibraryEx(L"threadpoolwinrt.dll", nullptr, 0);
}

bool XamlApp::Initialize(HINSTANCE hInstance) {
	FixThreadPoolCrash();

	ThemeHelper::Initialize();

	// 初始化 UWP 应用
	_uwpApp = winrt::XamlIslandsCpp::App::App();

	_mainWindow.Destroyed({ this, &XamlApp::_MainWindow_Destoryed });

	if (!_CreateMainWindow(hInstance)) {
		return false;
	}

	return true;
}

int XamlApp::Run() {
	return _mainWindow.MessageLoop();
}

bool XamlApp::_CreateMainWindow(HINSTANCE hInstance) noexcept {
	if (!_mainWindow.Create(hInstance)) {
		return false;
	}

	_uwpApp.HwndMain((uint64_t)_mainWindow.Handle());

	return true;
}

void XamlApp::_MainWindow_Destoryed() {
	_uwpApp.HwndMain(0);
}

}
