#pragma once
#include "pch.h"
#include "MainPage.g.h"


namespace winrt::XamlIslandsCpp::App::implementation
{
	struct MainPage : MainPageT<MainPage>
	{
		MainPage();

		~MainPage();
	};
}

namespace winrt::XamlIslandsCpp::App::factory_implementation
{
	struct MainPage : MainPageT<MainPage, implementation::MainPage>
	{
	};
}
