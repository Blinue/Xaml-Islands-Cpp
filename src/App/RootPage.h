#pragma once
#include "pch.h"
#include "RootPage.g.h"

namespace winrt::XamlIslandsCpp::App::implementation {

struct RootPage : RootPageT<RootPage> {
	void Loaded(IInspectable const&, RoutedEventArgs const&);

	void ClickHandler(IInspectable const&, RoutedEventArgs const&);
};

}

namespace winrt::XamlIslandsCpp::App::factory_implementation {

struct RootPage : RootPageT<RootPage, implementation::RootPage> {
};

}
