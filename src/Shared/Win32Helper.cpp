#include "pch.h"
#include "Win32Helper.h"

namespace XamlIslandsCpp {

static uint32_t GetOSBuild() noexcept {
	HMODULE hNtDll = GetModuleHandle(L"ntdll.dll");
	if (!hNtDll) {
		return 0;
	}

	auto rtlGetVersion = (LONG(WINAPI*)(PRTL_OSVERSIONINFOW))GetProcAddress(hNtDll, "RtlGetVersion");
	if (rtlGetVersion == nullptr) {
		assert(false);
		return 0;
	}

	RTL_OSVERSIONINFOW version{};
	version.dwOSVersionInfoSize = sizeof(version);
	rtlGetVersion(&version);

	return version.dwBuildNumber;
}

Win32Helper::OSVersion Win32Helper::GetOSVersion() noexcept {
	static OSVersion version = GetOSBuild();
	return version;
}

}
