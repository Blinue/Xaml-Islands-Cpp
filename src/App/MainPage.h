#pragma once
#include "pch.h"
#include "MainPage.g.h"


namespace winrt::XamlIslandsCpp::App::implementation {

struct MainPage : MainPageT<MainPage> {
	MainPage();

	void ClickHandler(IInspectable const&, RoutedEventArgs const&);
};

}

namespace winrt::XamlIslandsCpp::App::factory_implementation {

struct MainPage : MainPageT<MainPage, implementation::MainPage> {
};

}
