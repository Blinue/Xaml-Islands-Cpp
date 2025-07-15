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

}
