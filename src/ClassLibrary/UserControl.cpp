#include "pch.h"
#include "UserControl.h"
#if __has_include("UserControl.g.cpp")
#include "UserControl.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::ClassLibrary::implementation {
    void UserControl::ClickHandler(IInspectable const&, RoutedEventArgs const&)  {
        Button().Content(box_value(L"Clicked"));
    }
}
