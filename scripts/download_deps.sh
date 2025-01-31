#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 工具函数
print_step() {
    echo -e "${GREEN}==== $1 ====${NC}"
}

print_error() {
    echo -e "${RED}错误: $1${NC}"
}

# 初始化函数
init_env() {
    # 设置平台名称
    if [ "$(uname -m)" = "x86_64" ]; then
        PLATFORM_NAME="linux-x86_64"
    elif [ "$(uname -m)" = "aarch64" ]; then
        PLATFORM_NAME="linux-aarch64"
    else
        print_error "不支持的平台: $(uname -m)"
        exit 1
    fi

    # 设置安装路径
    INSTALL_PREFIX=$(realpath $(dirname $0)/../prebuild/${PLATFORM_NAME})
    BUILD_DIR=$(realpath $(dirname $0)/../build)
    
    # 创建必要的目录
    mkdir -p ${INSTALL_PREFIX}/{lib,include,bin}
    mkdir -p ${BUILD_DIR}
    
    print_step "安装目录: ${INSTALL_PREFIX}"
    print_step "构建目录: ${BUILD_DIR}"
}

# 依赖检查函数
check_dependencies() {
    local deps=("protobuf-compiler" "libprotobuf-dev" "cmake" "build-essential")
    
    for dep in "${deps[@]}"; do
        if ! dpkg -l | grep -q "^ii  $dep"; then
            print_step "安装 $dep"
            sudo apt-get update && sudo apt-get install -y "$dep"
            if [ $? -ne 0 ]; then
                print_error "安装 $dep 失败"
                exit 1
            fi
        fi
    done
}

# 库检查函数
check_lib_installed() {
    local lib_name=$1
    local lib_file=$2
    local header_file=$3
    
    if [ -f "${lib_file}" ] && [ -f "${header_file}" ]; then
        print_step "${lib_name} 已安装，跳过编译"
        return 0
    fi
    return 1
}

# Git克隆函数
git_clone_with_retry() {
    local repo_url=$1
    local branch=$2
    local target_dir=$3
    local max_retries=3
    local retry_count=0
    
    while [ $retry_count -lt $max_retries ]; do
        [ -d "$target_dir" ] && rm -rf "$target_dir"
        
        print_step "克隆 $target_dir (尝试 $((retry_count + 1))/$max_retries)"
        git clone --recursive -b $branch $repo_url $target_dir
        
        [ $? -eq 0 ] && return 0
        
        retry_count=$((retry_count + 1))
        [ $retry_count -lt $max_retries ] && sleep 5
    done
    
    print_error "克隆失败"
    return 1
}

# Protobuf编译函数
build_protobuf() {
    print_step "编译 Protobuf"
    cd ${BUILD_DIR}
    
    if ! git_clone_with_retry \
        "https://github.com/protocolbuffers/protobuf.git" \
        "v3.21.12" "protobuf"; then
        return 1
    fi
    
    cd protobuf
    mkdir -p build && cd build
    
    cmake ../cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} \
        -Dprotobuf_BUILD_TESTS=OFF \
        -Dprotobuf_BUILD_EXAMPLES=OFF \
        -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
        -Dprotobuf_BUILD_PROTOC=ON \
        -Dprotobuf_WITH_ZLIB=OFF \
        -DCMAKE_CXX_STANDARD=17 \
        -DCMAKE_CXX_FLAGS="-D_GLIBCXX_USE_CXX11_ABI=0"
        
    make -j$(nproc) && make install
    return $?
}

# ONNX Runtime编译函数
build_onnxruntime() {
    print_step "编译 ONNX Runtime"
    cd ${BUILD_DIR}
    
    if ! git_clone_with_retry \
        "https://github.com/microsoft/onnxruntime.git" \
        "v1.17.0" "onnxruntime"; then
        return 1
    fi
    
    cd onnxruntime
    
    export CFLAGS="-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0"
    export CXXFLAGS="-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0"
    export Protobuf_USE_STATIC_LIBS=ON
    export LDFLAGS="-L${INSTALL_PREFIX}/lib -Wl,-rpath,${INSTALL_PREFIX}/lib"
    
    ./build.sh \
        --config Release \
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
        return 1
    fi

    # 编译和安装
    cmake --build build/Linux/Release --target install -- -j$(nproc)
    
    if [ $? -ne 0 ]; then
        print_error "ONNX Runtime 编译失败"
        return 1
    fi

    print_step "ONNX Runtime 编译完成"
    return 0
}

# Poco编译函数
build_poco() {
    print_step "编译 Poco"
    cd ${BUILD_DIR}
    
    if ! git_clone_with_retry \
        "https://github.com/pocoproject/poco.git" \
        "poco-1.12.4-release" "poco"; then
        return 1
    fi
    
    cd poco
    mkdir -p build && cd build
    
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} \
        -DENABLE_TESTS=OFF \
        -DENABLE_SAMPLES=OFF
        
    make -j$(nproc) && make install
    return $?
}

# OpenCV编译函数
build_opencv() {
    print_step "编译 OpenCV"
    cd ${BUILD_DIR}
    
    if ! git_clone_with_retry \
        "https://github.com/opencv/opencv.git" \
        "4.8.0" "opencv"; then
        return 1
    fi
    
    cd opencv
    mkdir -p build && cd build
    
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} \
        -DBUILD_SHARED_LIBS=ON \
        -DBUILD_TESTS=OFF \
        -DBUILD_PERF_TESTS=OFF \
        -DBUILD_EXAMPLES=OFF \
        -DBUILD_opencv_apps=OFF \
        -DBUILD_LIST=core,imgproc,imgcodecs,videoio \
        -DWITH_FFMPEG=OFF \
        -DWITH_GTK=OFF \
        -DWITH_QT=OFF \
        -DWITH_CUDA=OFF \
        -DWITH_IPP=OFF \
        -DWITH_QUIRC=OFF \
        -DWITH_PROTOBUF=OFF
        
    make -j$(nproc) && make install
    return $?
}

# 主函数
main() {
    init_env
    check_dependencies
    
    # 编译OpenCV
    if ! check_lib_installed "OpenCV" \
        "${INSTALL_PREFIX}/lib/libopencv_core.so" \
        "${INSTALL_PREFIX}/include/opencv4/opencv2/core.hpp"; then
        build_opencv || exit 1
    fi
    
    # 编译Protobuf
    if ! check_lib_installed "Protobuf" \
        "${INSTALL_PREFIX}/lib/libprotobuf.a" \
        "${INSTALL_PREFIX}/include/google/protobuf/message.h"; then
        build_protobuf || exit 1
    fi
    
    # 编译ONNX Runtime
    if ! check_lib_installed "ONNX Runtime" \
        "${INSTALL_PREFIX}/lib/libonnxruntime.so" \
        "${INSTALL_PREFIX}/include/onnxruntime/onnxruntime_cxx_api.h"; then
        build_onnxruntime || exit 1
    fi
    
    # 编译Poco
    if ! check_lib_installed "Poco" \
        "${INSTALL_PREFIX}/lib/libPocoFoundation.so" \
        "${INSTALL_PREFIX}/lib/cmake/Poco/PocoConfig.cmake"; then
        build_poco || exit 1
    fi
    
    print_step "所有依赖编译完成"
}

# 执行主函数
main 