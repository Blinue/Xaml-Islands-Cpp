#pragma once

// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。


#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN
#define WINRT_LEAN_AND_MEAN
#define WINRT_NO_MODULE_LOCK

// Windows 头文件
#define NOMINMAX	// 使用 std::min 和 std::max 而不是宏
#include <windows.h>

// C++ 运行时头文件
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

// C++/WinRT 头文件
#include <unknwn.h>
#include <restrictederrorinfo.h>
#include <hstring.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Media.h>


namespace winrt {
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::UI::Xaml::Media;
}

