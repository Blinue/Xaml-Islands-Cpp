#pragma once

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
};

}
