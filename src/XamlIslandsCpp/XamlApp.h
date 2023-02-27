#pragma once
#include "pch.h"

#include "MainWindow.h"

struct IDesktopWindowXamlSourceNative2;

class XamlApp {
public:
	XamlApp(const XamlApp&) = delete;
	XamlApp(XamlApp&&) = delete;

	static XamlApp& Get() {
		static XamlApp instance;
		return instance;
	}

	bool Initialize(HINSTANCE hInstance);

	int Run();

private:
	XamlApp() = default;

	winrt::XamlIslandsCpp::App::App _uwpApp{ nullptr };
	MainWindow _mainWindow;
};
