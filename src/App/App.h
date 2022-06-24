#pragma once

#include "App.g.h"
#include "App.base.h"


namespace winrt::XamlIslandsCpp::App::implementation {

class App : public AppT2<App> {
public:
	App();
	~App();
};

}

namespace winrt::XamlIslandsCpp::App::factory_implementation {

class App : public AppT<App, implementation::App> {
};

}
