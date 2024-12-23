#include "pch.h"
#include "App.h"

using namespace winrt::XamlIslandsCpp::implementation;
using namespace winrt;

int APIENTRY wWinMain(
	_In_ HINSTANCE /*hInstance*/,
	_In_opt_ HINSTANCE /*hPrevInstance*/,
	_In_ LPWSTR /*lpCmdLine*/,
	_In_ int /*nCmdShow*/
) {
	// 程序结束时也不应调用 uninit_apartment
	// 见 https://kennykerr.ca/2018/03/24/cppwinrt-hosting-the-windows-runtime/
	init_apartment(apartment_type::single_threaded);

	App& app = App::Get();
	app.Initialize();
	return app.Run();
}
