#include "pch.h"
#include "App.h"

using namespace winrt::XamlIslandsCpp::implementation;

int APIENTRY wWinMain(
	_In_ HINSTANCE /*hInstance*/,
	_In_opt_ HINSTANCE /*hPrevInstance*/,
	_In_ LPWSTR /*lpCmdLine*/,
	_In_ int /*nCmdShow*/
) {
	winrt::init_apartment(winrt::apartment_type::single_threaded);

	App& app = App::Get();
	if (!app.Initialize()) {
		return 0;
	}

	return app.Run();
}
