#pragma once
#include "TitleBarControl.g.h"

namespace winrt::XamlIslandsCpp::implementation {

struct CaptionButtonsControl;

struct TitleBarControl : TitleBarControlT<TitleBarControl> {
	void IsWindowActive(bool value);

	CaptionButtonsControl& CaptionButtons() noexcept;
};

}

namespace winrt::XamlIslandsCpp::factory_implementation {

struct TitleBarControl : TitleBarControlT<TitleBarControl, implementation::TitleBarControl> {
};

}
