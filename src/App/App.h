#pragma once
#include "App.g.h"
#include "App.base.h"
#include <winrt/Windows.UI.Xaml.Hosting.h>

namespace winrt::XamlIslandsCpp::App::implementation {

class App : public AppT2<App> {
public:
	App();
	~App();

	void Close();

private:
	Hosting::WindowsXamlManager _windowsXamlManager{ nullptr };
	bool _isClosed = false;
};

}

namespace winrt::XamlIslandsCpp::App::factory_implementation {

class App : public AppT<App, implementation::App> {
};

}
