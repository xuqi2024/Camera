#!/bin/bash

# 删除旧的实现文件
rm -f src/video_capture.cpp src/gst_capture.cpp
rm -f include/video_capture.h include/gst_capture.h

# 清理构建目录
rm -rf build/
rm -rf prebuild/

# 清理模型文件，但保留.gitkeep
find models/ -type f ! -name '.gitkeep' -delete

# 重新创建必要的目录
mkdir -p cmake/modules cmake/toolchain
mkdir -p docs/architecture docs/api
mkdir -p include src models

# 设置文件权限
chmod +x build.sh clean.sh download_models.sh test.sh

echo "项目已重置" 