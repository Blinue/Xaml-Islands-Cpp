#pragma once
#include "TitleBarControl.g.h"

namespace winrt::XamlIslandsCpp::implementation {

struct TitleBarControl : TitleBarControlT<TitleBarControl> {
	void IsWindowActive(bool value);
};

}

namespace winrt::XamlIslandsCpp::factory_implementation {

struct TitleBarControl : TitleBarControlT<TitleBarControl, implementation::TitleBarControl> {
};

}
