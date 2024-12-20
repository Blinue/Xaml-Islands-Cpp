#include "pch.h"
#include "TitleBarControl.h"
#if __has_include("TitleBarControl.g.cpp")
#include "TitleBarControl.g.cpp"
#endif

namespace winrt::XamlIslandsCpp::implementation {

void TitleBarControl::IsWindowActive(bool value) {
	VisualStateManager::GoToState(*this, value ? L"Active" : L"NotActive", false);
	CaptionButtons().IsWindowActive(value);
}

}
