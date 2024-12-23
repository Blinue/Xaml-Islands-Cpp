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

	// std::format 会使二进制文件增大近 200KB
	std::wstring_view strView(templateStr);
	size_t replacementPos = strView.find(L"{}");
	std::wstring content(strView.substr(0, replacementPos));
	content.append(std::to_wstring(_count));
	content.append(strView.substr(replacementPos + 2));
	ClickMeButton().Content(box_value(content));
}

}
