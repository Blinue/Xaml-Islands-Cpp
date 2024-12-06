#include "pch.h"
#include "UserControl.h"
#if __has_include("UserControl.g.cpp")
#include "UserControl.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::ClassLibrary::implementation
{
    int32_t UserControl::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void UserControl::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void UserControl::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
