#include "pch.h"
#include "SmoothResizeHelper.h"
#include <inspectable.h>

// 来自 https://github.com/ALTaleX531/TranslucentFlyouts/blob/017970cbac7b77758ab6217628912a8d551fcf7c/TFModern/DiagnosticsHandler.cpp#L71-L84
DECLARE_INTERFACE_IID_(IFrameworkApplicationPrivate, IInspectable, "B3AB45D8-6A4E-4E76-A00D-32D4643A9F1A") {
	STDMETHOD(StartOnCurrentThread)(void* callback) PURE;
	STDMETHOD(CreateIsland)(void** island) PURE;
	STDMETHOD(CreateIslandWithAppWindow)(void* app_window, void** island) PURE;
	STDMETHOD(CreateIslandWithContentBridge)(void* owner, void* content_bridge, void** island) PURE;
	STDMETHOD(RemoveIsland)(void* island) PURE;
	STDMETHOD(SetSynchronizationWindow)(HWND hwnd) PURE;
};

namespace XamlIslandsCpp {

void SmoothResizeHelper::EnableResizeSync(HWND hWnd) noexcept {
	static auto enableResizeLayoutSynchronization = []() {
		HMODULE hUser32 = GetModuleHandle(L"user32");
		assert(hUser32);
		using tEnableResizeLayoutSynchronization = void(WINAPI*)(HWND hwnd, BOOL enable);
		return (tEnableResizeLayoutSynchronization)GetProcAddress(hUser32, MAKEINTRESOURCEA(2615));
	}();

	if (enableResizeLayoutSynchronization) {
		enableResizeLayoutSynchronization(hWnd, TRUE);
	}
}

void SmoothResizeHelper::SyncWindowSize(HWND hWnd, const winrt::Application& app) noexcept {
	// @apkipa 发现在 WM_SIZE 中调用 IFrameworkApplicationPrivate::SetSynchronizationWindow 可以防止闪烁。
	// 原理仍不清楚，似乎这个接口内部会调用 SynchronizedCommit，刚好实现了 UWP 调整大小的方法。
	// https://github.com/bigfatbrowncat/noflicker_directx_window/issues/1#issuecomment-2611975203
	if (auto appPrivate = app.try_as<IFrameworkApplicationPrivate>()) {
		appPrivate->SetSynchronizationWindow(hWnd);
	}
}

}
