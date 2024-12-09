#include "pch.h"
#include "UserControl.h"
#if __has_include("UserControl.g.cpp")
#include "UserControl.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace winrt::ClassLibrary::implementation {

static constexpr const wchar_t* RESOURCE_MAP_ID = L"ClassLibrary/Resources";

void UserControl::ClickHandler(IInspectable const& sender, RoutedEventArgs const&) {
	ResourceLoader resourceLoader = ResourceLoader::GetForCurrentView(RESOURCE_MAP_ID);
	sender.as<Button>().Content(box_value(resourceLoader.GetString(L"ClickMeButton_ClickedContent")));
}

}
