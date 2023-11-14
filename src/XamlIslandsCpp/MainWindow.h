#pragma once
#include "XamlWindow.h"
#include <winrt/XamlIslandsCpp.App.h>

class MainWindow : public XamlWindowT<MainWindow, winrt::XamlIslandsCpp::App::RootPage> {
	friend class base_type;
public:
	bool Initialize(HINSTANCE hInstance) noexcept;

protected:
	LRESULT _MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
};
