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
PLATFORM="linux-x86_64"
if [ "$2" = "aarch64" ]; then
    PLATFORM="linux-aarch64"
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
if [ ! -f "models/yolov4-tiny.weights" ]; then
    print_step "下载模型文件"
    ./download_models.sh
fi

# 编译第三方库（如果预编译目录不存在）
if [ ! -d "prebuild/${PLATFORM}/lib/cmake" ]; then
    print_step "编译第三方库"
    
    # 编译OpenCV
    print_step "编译OpenCV"
    mkdir -p build/opencv
    cd build/opencv
    cmake ../../3rdparty/opencv \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
        -DCMAKE_INSTALL_PREFIX=../../prebuild/${PLATFORM}
    make -j${NUM_CORES}
    make install
    cd ../..
    
    # 编译Poco
    print_step "编译Poco"
    mkdir -p build/poco
    cd build/poco
    cmake ../../3rdparty/poco \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
        -DCMAKE_INSTALL_PREFIX=../../prebuild/${PLATFORM}
    make -j${NUM_CORES}
    make install
    cd ../..

    # 编译onnx
    print_step "编译onnx"
    mkdir -p build/onnxruntime
    cd build/onnxruntime
    cmake ../../3rdparty/onnxruntime \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
        -DCMAKE_INSTALL_PREFIX=../../prebuild/${PLATFORM}
    make -j${NUM_CORES}
    make install
    cd ../..
else
    print_warning "使用预编译库"
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

# 在适当位置添加
if ! command -v wget &> /dev/null; then
    sudo apt-get install -y wget
fi

# 安装ONNX Runtime
install_onnxruntime() {
    print_step "安装ONNX Runtime"
    ONNX_VERSION="1.15.1"
    
    # 下载预编译包
    wget -c https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/onnxruntime-linux-x64-${ONNX_VERSION}.tgz
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}错误: 下载ONNX Runtime失败${NC}"
        echo "尝试使用备用下载链接..."
        wget -c https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/onnxruntime-linux-x64-gpu-${ONNX_VERSION}.tgz
    fi
    
    # 解压并安装
    tar xzf onnxruntime-linux-x64-*-${ONNX_VERSION}.tgz
    
    # 创建目录（如果不存在）
    sudo mkdir -p /usr/local/include/onnxruntime
    sudo mkdir -p /usr/local/lib
    
    # 复制文件
    sudo cp -r onnxruntime-linux-x64*/include/* /usr/local/include/onnxruntime/
    sudo cp -r onnxruntime-linux-x64*/lib/* /usr/local/lib/
    
    # 清理临时文件
    rm -rf onnxruntime-linux-x64*
    
    # 更新库缓存
    sudo ldconfig
    
    print_step "ONNX Runtime 安装完成"
}

# 检查ONNX Runtime是否已安装
if [ ! -f "/usr/local/include/onnxruntime/onnxruntime_cxx_api.h" ] || \
   [ ! -f "/usr/local/lib/libonnxruntime.so" ]; then
    install_onnxruntime
fi 