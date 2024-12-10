#pragma once
#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "UserControl.g.h"

namespace winrt::ClassLibrary::implementation {

struct UserControl : public UserControlT<UserControl> {
	void InitializeComponent();

	void ClickMeButton_Click(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);

private:
	void _UpdateClickMeButtonContent();

	static uint32_t _count;
};

}

namespace winrt::ClassLibrary::factory_implementation {

struct UserControl : UserControlT<UserControl, implementation::UserControl> {
};

}
