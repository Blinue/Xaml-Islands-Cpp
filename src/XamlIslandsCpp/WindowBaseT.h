#pragma once
#include <Windows.h>

namespace XamlIslandsCpp {

template <typename T>
class WindowBaseT {
public:
	using base_type = WindowBaseT<T>;

	WindowBaseT() noexcept = default;
	WindowBaseT(const WindowBaseT&) = delete;
	WindowBaseT(WindowBaseT&&) noexcept = default;

	HWND Handle() const noexcept {
		return _hWnd;
	}

	operator bool() const noexcept {
		return _hWnd;
	}

	void Destroy() noexcept {
		if (_hWnd) {
			DestroyWindow(_hWnd);
		}
	}

protected:
	// 析构函数为 protected 使得无法通过基类指针删除。这里不调用 Destroy，因为父类的
	// 析构函数在子类析构之后才会执行。
	~WindowBaseT() noexcept {}

	static LRESULT CALLBACK _WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
		if (msg == WM_NCCREATE) {
			WindowBaseT* that = (WindowBaseT*)(((CREATESTRUCT*)lParam)->lpCreateParams);
			assert(that && !that->_hWnd);
			that->_hWnd = hWnd;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)that);
		} else if (T* that = (T*)GetWindowLongPtr(hWnd, GWLP_USERDATA)) {
			return that->_MessageHandler(msg, wParam, lParam);
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	LRESULT _MessageHandler(UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
		switch (msg) {
		case WM_DESTROY:
		{
			_hWnd = NULL;
			return 0;
		}
		}

		return DefWindowProc(_hWnd, msg, wParam, lParam);
	}

private:
	HWND _hWnd = NULL;
};

}
