#pragma once
#include "TitleBarControl.g.h"

namespace winrt::XamlIslandsCpp::App::implementation {

struct TitleBarControl : TitleBarControlT<TitleBarControl> {
	void IsWindowActive(bool value);
};

}

namespace winrt::XamlIslandsCpp::App::factory_implementation {

struct TitleBarControl : TitleBarControlT<TitleBarControl, implementation::TitleBarControl> {
};

}
