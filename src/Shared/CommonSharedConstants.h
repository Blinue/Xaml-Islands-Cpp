#pragma once
#include <Windows.h>

namespace XamlIslandsCpp {

struct CommonSharedConstants {
	static constexpr const wchar_t* MAIN_WINDOW_CLASS_NAME = L"XamlIslandsCpp_Main";
	static constexpr const wchar_t* TITLE_BAR_WINDOW_CLASS_NAME = L"XamlIslandsCpp_TitleBar";

	static constexpr const COLORREF LIGHT_TINT_COLOR = RGB(243, 243, 243);
	static constexpr const COLORREF DARK_TINT_COLOR = RGB(32, 32, 32);

	static constexpr const wchar_t* APP_RESOURCE_MAP_ID = L"XamlIslandsCpp.App/Resources";
};

}
