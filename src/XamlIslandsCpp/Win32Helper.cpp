#include "pch.h"
#include "Win32Helper.h"

namespace XamlIslandsCpp {

Win32Helper::OSVersion Win32Helper::GetOSVersion() noexcept {
	static OSVersion version = [] {
		const auto rtlGetVersion =
			LoadSystemFunction<LONG WINAPI(PRTL_OSVERSIONINFOW)>(L"ntdll.dll", "RtlGetVersion");
		if (!rtlGetVersion) {
			return OSVersion();
		}

		RTL_OSVERSIONINFOW versionInfo{ .dwOSVersionInfoSize = sizeof(versionInfo) };
		rtlGetVersion(&versionInfo);

		return OSVersion(versionInfo.dwBuildNumber);
	}();

	return version;
}

bool Win32Helper::GetClientScreenRect(HWND hWnd, RECT& rect) noexcept {
	if (!GetClientRect(hWnd, &rect)) {
		return false;
	}

	POINT p{};
	if (!ClientToScreen(hWnd, &p)) {
		return false;
	}

	rect.bottom += p.y;
	rect.left += p.x;
	rect.right += p.x;
	rect.top += p.y;

	return true;
}

}
