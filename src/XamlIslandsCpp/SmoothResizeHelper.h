#pragma once

namespace XamlIslandsCpp {

struct SmoothResizeHelper {
	// 初始化 XAML Islands 后调用
	static void EnableResizeSync(HWND hWnd) noexcept;

	// 调整 CoreWindow 尺寸前调用
	static void SyncWindowSize(HWND hWnd, const winrt::Application& app) noexcept;
};

}
