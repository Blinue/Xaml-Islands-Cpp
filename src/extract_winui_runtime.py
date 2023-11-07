import sys
import os
import glob
import zipfile

if len(sys.argv) != 2:
    raise Exception("请勿直接运行此脚本")

platform = sys.argv[1]
if not platform in ["x64", "ARM64"]:
    raise Exception("非法参数")

os.chdir(os.path.dirname(__file__) + "\\..\\packages")
packagesFolder = os.getcwd()

winuiPkg = max(glob.glob("Microsoft.UI.Xaml*"))

intDir = f"..\\obj\\{platform}\\WinUI"
os.makedirs(intDir, exist_ok=True)
os.chdir(intDir)

needExtract = True

try:
    with open("version.txt") as f:
        if f.read() == winuiPkg:
            needExtract = False
except:
    pass

if needExtract:
    with zipfile.ZipFile(
        max(
            glob.glob(
                f"{packagesFolder}\\{winuiPkg}\\tools\\AppX\\{platform}\\Release\\Microsoft.UI.Xaml*.appx"
            )
        )
    ) as appx:
        # 收集要解压的文件
        members = ["Microsoft.UI.Xaml.dll", "resources.pri"]
        for file in appx.namelist():
            if file.startswith("Microsoft.UI.Xaml"):
                members.append(file)

        appx.extractall(members=members)

    with open("version.txt", mode="w") as f:
        f.write(winuiPkg)
