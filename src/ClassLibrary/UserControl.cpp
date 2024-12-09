#include "pch.h"
#include "UserControl.h"
#if __has_include("UserControl.g.cpp")
#include "UserControl.g.cpp"
#endif
#include <format>

using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

namespace winrt::ClassLibrary::implementation {

static constexpr const wchar_t* RESOURCE_MAP_ID = L"ClassLibrary/Resources";

uint32_t UserControl::_count = 0;

void UserControl::InitializeComponent() {
	UserControlT::InitializeComponent();

	if (_count != 0) {
		_UpdateClickMeButtonContent();
	}
}

void UserControl::ClickMeButton_Click(IInspectable const&, RoutedEventArgs const&) {
	++_count;
	_UpdateClickMeButtonContent();
}

void UserControl::_UpdateClickMeButtonContent() {
	ResourceLoader resourceLoader = ResourceLoader::GetForCurrentView(RESOURCE_MAP_ID);
	hstring templateStr = resourceLoader.GetString(L"ClickMeButton_ClickedContent");
	ClickMeButton().Content(box_value(std::vformat(templateStr, std::make_wformat_args(_count))));
}

}
