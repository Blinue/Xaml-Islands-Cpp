#include "pch.h"
#include "MainPage.h"
#if __has_include("MainPage.g.cpp")
#include "MainPage.g.cpp"
#endif

namespace winrt::XamlIslandsCpp::App::implementation {

void MainPage::ClickHandler(IInspectable const&, RoutedEventArgs const&) {
	Button().Content(box_value(L"BOOM!"));
}

}
