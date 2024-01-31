import sys
import os
import glob
import zipfile
import shutil

if len(sys.argv) != 3:
    raise Exception("请勿直接运行此脚本")

platform = sys.argv[1]
if not platform in ["x64", "ARM64"]:
    raise Exception("非法参数")

# sys.argv[2] 为 winmd 路径
winuiWinmdPath = sys.argv[2]
if not winuiWinmdPath.endswith("\\lib\\uap10.0\\Microsoft.UI.Xaml.winmd"):
    raise Exception("非法参数")
winuiPkgDir = winuiWinmdPath[: -len("\\lib\\uap10.0\\Microsoft.UI.Xaml.winmd")]
winuiPkgId = winuiPkgDir[winuiPkgDir.rfind("\\") + 1 :]

intDir = os.path.dirname(__file__) + f"\\..\\obj\\{platform}\\WinUI"

if "prerelease" in winuiPkgId:
    # 预览版本的 WinUI 无需解压
    shutil.rmtree(intDir, ignore_errors=True)
else:
    os.makedirs(intDir, exist_ok=True)
    os.chdir(intDir)

    def needExtract():
        try:
            with open("version.txt") as f:
                if f.read() != winuiPkgId:
                    return True

            for path in (
                "Microsoft.UI.Xaml.dll",
                "Microsoft.UI.Xaml.pri",
            ):
                if not os.access(path, os.F_OK):
                    return True
        except:
            return True

        return False

    if needExtract():
        with zipfile.ZipFile(
            glob.glob(f"{winuiPkgDir}\\tools\\AppX\\{platform}\\Release\\*.appx")[0]
        ) as appx:
            appx.extractall(members=("Microsoft.UI.Xaml.dll", "resources.pri"))

        # 将 resources.pri 重命名为 Microsoft.UI.Xaml.pri
        try:
            os.remove("Microsoft.UI.Xaml.pri")
        except:
            pass
        os.rename("resources.pri", "Microsoft.UI.Xaml.pri")

        with open("version.txt", mode="w") as f:
            f.write(winuiPkgId)
