import sys
import os

if len(sys.argv) != 2:
    raise Exception("请勿直接运行此脚本")

with open(sys.argv[1], "r+") as f:
    lines = []
    for line in f.readlines():
        if line.find("\\packages\\Microsoft.UI.Xaml") == -1:
            lines.append(line)

    f.seek(os.SEEK_SET)
    f.truncate()
    f.writelines(lines)
