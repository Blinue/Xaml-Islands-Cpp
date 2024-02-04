#pragma once
#include "pch.h"
#include "RootPage.g.h"

namespace winrt::XamlIslandsCpp::App::implementation {

struct RootPage : RootPageT<RootPage> {
};

}

namespace winrt::XamlIslandsCpp::App::factory_implementation {

struct RootPage : RootPageT<RootPage, implementation::RootPage> {
};

}
