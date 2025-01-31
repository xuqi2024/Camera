#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_step() {
    echo -e "${GREEN}==== $1 ====${NC}"
}

print_error() {
    echo -e "${RED}错误: $1${NC}"
}

# 设置编译参数
ONNX_VERSION="1.17.0"
BUILD_TYPE="Release"
# 设置平台名称
if [ "$(uname -m)" = "x86_64" ]; then
    PLATFORM_NAME="linux-x86_64"
elif [ "$(uname -m)" = "aarch64" ]; then
    PLATFORM_NAME="linux-aarch64"
else
    print_error "不支持的平台: $(uname -m)"
    exit 1
fi

INSTALL_PREFIX="$(pwd)/prebuild/${PLATFORM_NAME}"

# 创建目录
mkdir -p ${INSTALL_PREFIX}/{lib,include}
mkdir -p build/onnxruntime
cd build/onnxruntime

# 下载源码
print_step "下载 ONNX Runtime 源码"
if [ ! -d "onnxruntime" ]; then
    git clone --recursive --branch v${ONNX_VERSION} --depth 1 https://github.com/microsoft/onnxruntime.git
fi

cd onnxruntime

# 配置编译选项
print_step "配置 ONNX Runtime"
# 禁用 _FORTIFY_SOURCE
export CFLAGS="-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0"
export CXXFLAGS="-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0"
# 使用prebuild的protobuf
# 设置protobuf相关环境变量
export Protobuf_USE_STATIC_LIBS=ON
export LDFLAGS="-L${INSTALL_PREFIX}/lib -Wl,-rpath,${INSTALL_PREFIX}/lib"

./build.sh \
    --config ${BUILD_TYPE} \
    --build_shared_lib \
    --parallel \
    --cmake_extra_defines \
        CMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} \
        CMAKE_POSITION_INDEPENDENT_CODE=ON \
        CMAKE_CXX_FLAGS="-Wno-error -Wno-error=stringop-overflow -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -D_GLIBCXX_USE_CXX11_ABI=0" \
        CMAKE_C_FLAGS="-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0" \
        CMAKE_EXE_LINKER_FLAGS="-L${INSTALL_PREFIX}/lib -lprotobuf" \
        CMAKE_SHARED_LINKER_FLAGS="-L${INSTALL_PREFIX}/lib -lprotobuf" \
        CMAKE_INSTALL_LIBDIR=lib \
        CMAKE_INSTALL_INCLUDEDIR=include \
        Protobuf_ROOT=${INSTALL_PREFIX} \
        Protobuf_INCLUDE_DIR=${INSTALL_PREFIX}/include \
        Protobuf_LIBRARY=${INSTALL_PREFIX}/lib/libprotobuf.a \
        Protobuf_PROTOC_EXECUTABLE=${INSTALL_PREFIX}/bin/protoc \
        Protobuf_USE_STATIC_LIBS=ON \
        Protobuf_LIBRARIES=${INSTALL_PREFIX}/lib/libprotobuf.a \
        protobuf_BUILD_SHARED_LIBS=OFF \
        CMAKE_POLICY_DEFAULT_CMP0069=NEW \
        CMAKE_POLICY_DEFAULT_CMP0148=OLD \
        onnxruntime_BUILD_SHARED_LIB=ON \
        onnxruntime_BUILD_UNIT_TESTS=OFF \
        onnxruntime_USE_CUDA=OFF \
        onnxruntime_USE_TENSORRT=OFF \
        onnxruntime_USE_OPENVINO=OFF \
        onnxruntime_USE_DNNL=OFF \
        onnxruntime_USE_NNAPI=OFF \
        onnxruntime_USE_ROCM=OFF \
        onnxruntime_USE_NUPHAR=OFF \
        onnxruntime_MINIMAL_BUILD=ON \
        onnxruntime_DISABLE_RTTI=OFF \
        onnxruntime_DISABLE_EXCEPTIONS=OFF \
        onnxruntime_DISABLE_ABSEIL=ON \
        onnxruntime_DISABLE_PYTHON=ON \
        onnxruntime_DISABLE_EXTERNAL_INITIALIZERS=ON \
        onnxruntime_USE_PREBUILT_PB=ON \
        onnxruntime_USE_EIGEN_FOR_BLAS=ON

if [ $? -ne 0 ]; then
    print_error "ONNX Runtime 配置失败"
    exit 1
fi

# 编译和安装
print_step "编译 ONNX Runtime"
cmake --build build/Linux/${BUILD_TYPE} --target install -- -j$(nproc)

if [ $? -ne 0 ]; then
    print_error "ONNX Runtime 编译失败"
    exit 1
fi

print_step "ONNX Runtime 编译完成"
echo "安装目录: ${INSTALL_PREFIX}"

# 验证安装
if [ -f "${INSTALL_PREFIX}/lib/libonnxruntime.so" ] && \
   [ -f "${INSTALL_PREFIX}/include/onnxruntime/onnxruntime_cxx_api.h" ]; then
    echo -e "${GREEN}ONNX Runtime 安装成功${NC}"
else
    print_error "ONNX Runtime 安装验证失败"
    exit 1
fi 