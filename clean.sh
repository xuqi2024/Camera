#!/bin/bash

# 清理构建目录
# 删除所有编译生成的文件和目录
rm -rf build/          # 删除CMake构建目录
rm -rf prebuild/       # 删除预编译库目录

# 清理模型文件
# 删除下载的AI模型文件
rm -f models/*.onnx    # 删除ONNX模型文件
rm -f models/*.pt      # 删除PyTorch模型文件

# 清理编译产物
# 使用find命令递归删除所有编译生成的文件
find . -name "*.o" -delete     # 删除目标文件
find . -name "*.so" -delete    # 删除共享库
find . -name "*.a" -delete     # 删除静态库

# 清理日志文件
find . -name "*.log" -delete   # 删除所有日志文件

# 清理IDE配置
# 删除IDE生成的配置文件和目录
rm -rf .vscode/                # 删除VSCode配置
rm -rf .idea/                  # 删除CLion配置

echo "清理完成" 