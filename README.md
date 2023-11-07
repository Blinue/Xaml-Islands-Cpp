# Xaml-Islands-Cpp

![MainWindow.png](img/MainWindow.png)

本项目实现了一个最小的 [XAML Islands](https://docs.microsoft.com/en-us/windows/apps/desktop/modernize/xaml-islands) (C++/WinRT) 应用。目标是：

1. 作为创建 XAML Islands 应用的起点。
2. 作为一个用于实验/调试的最小环境。

**已实现的功能**

:heavy_check_mark: XAML Islands 集成

:heavy_check_mark: WinUI 集成（支持正式版而不是仅限预发行版）

:heavy_check_mark: 焦点处理

:heavy_check_mark: 响应 DPI 更改

:heavy_check_mark: 集成 [HybridCRT](https://github.com/microsoft/WindowsAppSDK/blob/main/docs/Coding-Guidelines/HybridCRT.md)，不依赖 Visual C++ 运行时

:heavy_check_mark: [修剪 resources.pri](https://github.com/microsoft/microsoft-ui-xaml/pull/4400)

**不会实现的功能**（这些功能超出了本项目的目标，你可在 [Magpie](https://github.com/Blinue/Magpie) 中找到它们的实现）

:x: 明暗主题切换

:x: Mica 背景

:x: 自定义标题栏

## 编译要求

1. Windows SDK 22621
2. Visual Studio 2022，需安装 C++ 和 UWP 负载

## 运行要求

Windows 10 v1903+ 或 Windows 11

## 发布

执行 publish.py 将编译程序、清理文件以及修剪 resources.pri，可用于发布的程序体积约 8.35MB。
