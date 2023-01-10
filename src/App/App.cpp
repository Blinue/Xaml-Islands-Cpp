#include "pch.h"
#include "App.h"
#if __has_include("App.g.cpp")
#include "App.g.cpp"
#endif

namespace winrt::XamlIslandsCpp::App::implementation {

App::App() {
	// 初始化 XAML 框架
	_windowsXamlManager = Hosting::WindowsXamlManager::InitializeForCurrentThread();
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
