#pragma once
#include "pch.h"
#include "MainPage.g.h"

namespace winrt::XamlIslandsCpp::App::implementation {

struct MainPage : MainPageT<MainPage> {
	void Loaded(IInspectable const&, RoutedEventArgs const&);

	void ClickHandler(IInspectable const&, RoutedEventArgs const&);
};

}

namespace winrt::XamlIslandsCpp::App::factory_implementation {

struct MainPage : MainPageT<MainPage, implementation::MainPage> {
};

}
