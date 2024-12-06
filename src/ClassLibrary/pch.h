#pragma once

#include <SDKDDKVer.h>

// Windows 头文件
#include <windows.h>
#include <windowsx.h>

// 修复 C++/WinRT 头文件的警告
#undef GetCurrentTime

#include <unknwn.h>
#include <restrictederrorinfo.h>
#include <hstring.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>
