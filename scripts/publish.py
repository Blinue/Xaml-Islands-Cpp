import sys
import os
import subprocess
import shutil
import glob
import argparse

try:
    # https://docs.github.com/en/actions/learn-github-actions/variables
    if os.environ["GITHUB_ACTIONS"].lower() == "true":
        # 不知为何在 Github Actions 中运行时默认编码为 ANSI，并且 print 需刷新流才能正常显示
        for stream in [sys.stdout, sys.stderr]:
            stream.reconfigure(encoding="utf-8")
except:
    pass

argParser = argparse.ArgumentParser()
argParser.add_argument("--compiler", choices=["MSVC", "ClangCL"], default="MSVC")
argParser.add_argument("--platform", choices=["x64", "ARM64"], default="x64")
argParser.add_argument("--use-native-march", action="store_true")
argParser.add_argument("--packaged", action="store_true")
args = argParser.parse_args()

#####################################################################
#
# 使用 vswhere 查找 msbuild
#
#####################################################################

programFilesX86Path = os.environ["ProgramFiles(x86)"]
vswherePath = programFilesX86Path + "\\Microsoft Visual Studio\\Installer\\vswhere.exe"
if not os.access(vswherePath, os.X_OK):
    raise Exception("未找到 vswhere")

p = subprocess.run(
    vswherePath
    + " -latest -requires Microsoft.Component.MSBuild -find MSBuild\\**\\Bin\\MSBuild.exe",
    capture_output=True,
)
msbuildPath = str(p.stdout, encoding="utf-8").splitlines()[0]
if not os.access(msbuildPath, os.X_OK):
    raise Exception("未找到 msbuild")

#####################################################################
#
# 编译
#
#####################################################################

curDir = os.path.normpath(os.path.dirname(__file__) + "\\..\\")
os.chdir(curDir)

outDir = f"{curDir}\\publish\\{args.platform}{'-sideload' if args.packaged else ''}\\"

p = subprocess.run(
    f'"{msbuildPath}" src\\{"AppPackage" if args.packaged else "XamlIslandsCpp"} -m -t:Rebuild -restore "-p:RestorePackagesConfig=true;Configuration=Release;Platform={args.platform};DisablePDB=true;UseClangCL={args.compiler == "ClangCL"};UseNativeMicroArch={args.use_native_march};IsPackaged={args.packaged};OutDir={outDir};SolutionDir={curDir}\\{(";AppInstallerUri=" + outDir) if args.packaged else ""}"'
)
if p.returncode != 0:
    raise Exception("编译失败")


#####################################################################
#
# 清理不需要的文件
#
#####################################################################

os.chdir(outDir)

# 删除文件，忽略错误
def remove_file(file):
    try:
        os.remove(file)
    except:
        pass

if args.packaged:
    msixPath = glob.glob("AppPackage\\AppPackages\\AppPackage_*\\AppPackage_*.msix")[0]
    cerPath = glob.glob("AppPackage\\AppPackages\\AppPackage_*\\AppPackage_*.cer")[0]
    msixName = f"XamlIslandsCpp-{args.platform}.msix"
    cerName = "TemporaryKey.cer"
    remove_file(msixName)
    remove_file(cerName)
    os.rename(msixPath, msixName)
    os.rename(cerPath, cerName)

    # 删除其他文件
    shutil.rmtree("AppPackage", ignore_errors=True)
    for file in os.listdir():
        if not file.endswith(".msix") and not file.endswith(".cer"):
            remove_file(file)
else:
    for pattern in ["*.lib", "*.exp"]:
        for file in glob.glob(pattern):
            remove_file(file)
    
    for file in glob.glob("*.pri"):
        if file != "resources.pri":
            remove_file(file)

print("清理完毕", flush=True)
