#include "pch.h"
#include "RootPage.h"
#if __has_include("RootPage.g.cpp")
#include "RootPage.g.cpp"
#endif

namespace winrt::XamlIslandsCpp::App::implementation {

void RootPage::Loaded(IInspectable const&, RoutedEventArgs const&) {
	// 修复 WinUI 的汉堡菜单的尺寸 bug
	RootNavigationView().PaneDisplayMode(MUXC::NavigationViewPaneDisplayMode::Auto);

	// 消除焦点框
	IsTabStop(true);
	Focus(FocusState::Programmatic);
	IsTabStop(false);
}

void RootPage::ClickHandler(IInspectable const&, RoutedEventArgs const&) {
	Button().Content(box_value(L"BOOM!"));
}

}
