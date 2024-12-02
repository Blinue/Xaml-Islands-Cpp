#include "pch.h"
#include "ThemeHelper.h"
#include <winrt/XamlIslandsCpp.h>
#include "App.h"
#include "MainWindow.h"

using namespace XamlIslandsCpp;

// 提前加载 twinapi.appcore.dll 和 threadpoolwinrt.dll 以避免退出时崩溃。应在 Windows.UI.Xaml.dll 被加载前调用
// 来自 https://github.com/CommunityToolkit/Microsoft.Toolkit.Win32/blob/6fb2c3e00803ea563af20f6bc9363091b685d81f/Microsoft.Toolkit.Win32.UI.XamlApplication/XamlApplication.cpp#L140
// 参见：https://github.com/microsoft/microsoft-ui-xaml/issues/7260#issuecomment-1231314776
static void FixThreadPoolCrash() noexcept {
	LoadLibraryEx(L"twinapi.appcore.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	LoadLibraryEx(L"threadpoolwinrt.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
}

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE /*hPrevInstance*/,
	_In_ LPWSTR /*lpCmdLine*/,
	_In_ int /*nCmdShow*/
) {
	// 程序结束时也不应调用 uninit_apartment
	// 见 https://kennykerr.ca/2018/03/24/cppwinrt-hosting-the-windows-runtime/
	winrt::init_apartment(winrt::apartment_type::single_threaded);

	FixThreadPoolCrash();

	ThemeHelper::Initialize();

	// 初始化 UWP 应用。退出时不要调用 Close，如果正在播放动画会崩溃
	auto uwpApp = winrt::make_self<winrt::XamlIslandsCpp::implementation::App>();

	if (!MainWindow::Get().Create(hInstance)) {
		return -1;
	}

	return MainWindow::Get().MessageLoop();
}
