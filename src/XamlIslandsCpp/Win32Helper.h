#pragma once

// 来自 WIL
// Retrieve the HINSTANCE for the current DLL or EXE using this symbol that
// the linker provides for every module. This avoids the need for a global
// HINSTANCE variable and provides access to this value for static libraries.
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace XamlIslandsCpp {

struct Win32Helper {
	struct OSVersion {
		constexpr OSVersion(uint32_t build) : _build(build) {}

		bool Is20H1OrNewer() const noexcept {
			return _build >= 19041;
		}

		// 下面为 Win11
		// 不考虑代号相同的 Win10

		bool IsWin11() const noexcept {
			return Is21H2OrNewer();
		}

		bool Is21H2OrNewer() const noexcept {
			return _build >= 22000;
		}

		bool Is22H2OrNewer() const noexcept {
			return _build >= 22621;
		}

	private:
		uint32_t _build = 0;
	};

	static OSVersion GetOSVersion() noexcept;

	static HINSTANCE GetModuleInstanceHandle() noexcept {
		return reinterpret_cast<HINSTANCE>(&__ImageBase);
	}
};

}
