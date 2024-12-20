#pragma once
#include "TitleBarControl.g.h"
#include "CaptionButtonsControl.h"

namespace winrt::XamlIslandsCpp::implementation {

struct TitleBarControl : TitleBarControlT<TitleBarControl> {
	void IsWindowActive(bool value);

	CaptionButtonsControl& CaptionButtons() noexcept {
		return *get_self<CaptionButtonsControl>(TitleBarControlT::CaptionButtons());
	}
};

}

namespace winrt::XamlIslandsCpp::factory_implementation {

struct TitleBarControl : TitleBarControlT<TitleBarControl, implementation::TitleBarControl> {
};

}
