#include "pch.h"
#include "RootPage.h"
#if __has_include("RootPage.g.cpp")
#include "RootPage.g.cpp"
#endif

namespace winrt::XamlIslandsCpp::App::implementation {

void RootPage::Loaded(IInspectable const&, RoutedEventArgs const&) {
	// 消除焦点框
	IsTabStop(true);
	Focus(FocusState::Programmatic);
	IsTabStop(false);
}

void RootPage::ClickHandler(IInspectable const&, RoutedEventArgs const&) {
	ResourceLoader resourceLoader = ResourceLoader::GetForCurrentView(L"XamlIslandsCpp.App/Resources");
	Button().Content(box_value(resourceLoader.GetString(L"TestText")));
}

}
