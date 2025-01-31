#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 错误处理
set -e
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
trap 'if [ $? -ne 0 ]; then echo -e "${RED}错误: 命令 ${last_command} 失败${NC}"; fi' EXIT

# 工具函数
print_step() {
    echo -e "${GREEN}==== $1 ====${NC}"
}

print_warning() {
    echo -e "${YELLOW}警告: $1${NC}"
}

print_error() {
    echo -e "${RED}错误: $1${NC}"
}

# 检查必要的命令
check_command() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}错误: 未找到命令 $1${NC}"
        echo "请安装必要的依赖:"
        echo "sudo apt-get update && sudo apt-get install -y $2"
        exit 1
    fi
}

check_command "git" "git"
check_command "cmake" "cmake"
check_command "make" "build-essential"

# 初始化子模块
print_step "初始化和更新子模块"
git submodule init
git submodule update --init --recursive

# 确保子模块在正确的版本上
print_step "检出指定版本"
cd 3rdparty/opencv
git checkout 4.8.0
cd ../poco
git checkout poco-1.12.4-release
cd ../..

# 确定目标平台
if [ "$(uname -m)" = "aarch64" ]; then
    PLATFORM="linux-aarch64"
else
    PLATFORM="linux-x86_64"
fi

# 创建目录
print_step "创建目录结构"
mkdir -p src include 3rdparty build
mkdir -p prebuild/${PLATFORM}/{include,lib}

# 配置编译类型
BUILD_TYPE="Release"
if [ "$1" = "debug" ]; then
    BUILD_TYPE="Debug"
fi

# 检测CPU核心数
NUM_CORES=$(nproc)

# 下载模型文件
if [ ! -f "models/yolov11n.onnx" ]; then
    print_step "下载模型文件"
    ./download_models.sh
fi

# 在build.sh开头添加
check_prebuild() {
    local platform=$1
    echo "检查预编译库目录: prebuild/${platform}"
    
    # 检查必要的目录和文件
    local required_paths=(
        "prebuild/${platform}/include/onnxruntime/onnxruntime_cxx_api.h"
        "prebuild/${platform}/lib/libonnxruntime.so"
        "prebuild/${platform}/lib/cmake/opencv4"
        "prebuild/${platform}/lib/cmake/Poco"
    )
    
    for path in "${required_paths[@]}"; do
        if [ ! -e "$path" ]; then
            echo "缺少文件: $path"
            return 1
        fi
    done
    
    return 0
}

# 检查预编译库
print_step "检查预编译库"
if check_prebuild ${PLATFORM}; then
    print_warning "使用预编译库"
    BUILD_FROM_SOURCE=false
else
    print_step "预编译库不完整，将从源码构建"
    BUILD_FROM_SOURCE=true
fi

# 如果需要从源码构建
if [ "$BUILD_FROM_SOURCE" = true ]; then
    # 设置PLATFORM_NAME变量
    export PLATFORM_NAME=${PLATFORM}
    
    # 运行下载和编译脚本
    print_step "下载和编译依赖"
    # 确保scripts目录存在
    if [ ! -d "scripts" ]; then
        print_error "scripts目录不存在"
        exit 1
    fi
    
    # 运行下载脚本
    ./scripts/download_deps.sh
    if [ $? -ne 0 ]; then
        print_error "下载依赖失败"
        exit 1
    fi
else
    print_warning "使用预编译库，跳过下载和编译"
fi

# 编译项目
print_step "编译项目"
cd build
if [ "$2" = "aarch64" ]; then
    cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
          -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain/aarch64-linux-gnu.cmake ..
else
    cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..
fi

make -j${NUM_CORES}

# 检查编译结果
if [ -f "bin/video_streaming_app" ]; then
    print_step "编译成功!"
    echo -e "${GREEN}可执行文件位置: $(pwd)/bin/video_streaming_app${NC}"
else
    echo -e "${RED}编译失败!${NC}"
    exit 1
fi 