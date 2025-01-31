cmake_minimum_required(VERSION 3.10)
project(video_streaming_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Threads包
find_package(Threads REQUIRED)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 确定目标平台
if(CMAKE_CROSSCOMPILING)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(PLATFORM_NAME "linux-aarch64")
    else()
        message(FATAL_ERROR "Unsupported platform")
    endif()
else()
    set(PLATFORM_NAME "linux-x86_64")
endif()

# 设置预编译目录
set(PREBUILD_DIR ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME})
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/3rdparty)

# 添加cmake模块路径
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# 设置查找路径
set(CMAKE_PREFIX_PATH ${PREBUILD_DIR} ${CMAKE_PREFIX_PATH})

# 检查预编译库是否存在
if(NOT EXISTS ${PREBUILD_DIR})
    message(STATUS "Prebuild directory not found, will build from source")
    set(BUILD_FROM_SOURCE TRUE)
endif()

# 在开始位置添加调试信息
message(STATUS "PLATFORM_NAME: ${PLATFORM_NAME}")
message(STATUS "PREBUILD_DIR: ${PREBUILD_DIR}")

if(BUILD_FROM_SOURCE)
    # OpenCV配置
    set(BUILD_LIST core imgproc objdetect videoio)
    set(WITH_FFMPEG OFF)
    set(WITH_GTK OFF)
    set(WITH_QT OFF)
    set(BUILD_TESTS OFF)
    set(BUILD_PERF_TESTS OFF)
    set(BUILD_opencv_apps OFF)
    set(BUILD_EXAMPLES OFF)
    set(WITH_PROTOBUF OFF)
    set(WITH_QUIRC OFF)
    set(WITH_CUDA OFF)
    
    add_subdirectory(${THIRD_PARTY_DIR}/opencv)
    
    # Poco配置
    set(ENABLE_FOUNDATION ON)
    set(ENABLE_NET ON)
    set(ENABLE_NETSSL OFF)
    set(ENABLE_CRYPTO OFF)
    set(ENABLE_JWT OFF)
    set(ENABLE_DATA OFF)
    set(ENABLE_UTIL OFF)
    set(ENABLE_XML OFF)
    set(ENABLE_JSON ON)
    set(ENABLE_MONGODB OFF)
    set(ENABLE_REDIS OFF)
    
    add_subdirectory(${THIRD_PARTY_DIR}/poco)

    # ONNX Runtime配置
    set(ONNXRUNTIME_BUILD_SHARED_LIB ON)
    set(ONNXRUNTIME_BUILD_UNIT_TESTS OFF)
    set(ONNXRUNTIME_BUILD_BENCHMARKS OFF)
    set(ONNXRUNTIME_USE_CUDA OFF)
    set(ONNXRUNTIME_USE_TENSORRT OFF)
    set(ONNXRUNTIME_USE_OPENVINO OFF)
    set(ONNXRUNTIME_USE_DNNL OFF)
    set(ONNXRUNTIME_USE_NUPHAR OFF)
    set(ONNXRUNTIME_USE_EIGEN_FOR_BLAS ON)
    
    add_subdirectory(${THIRD_PARTY_DIR}/onnxruntime)
else()
    # 使用预编译库
    set(OpenCV_DIR ${PREBUILD_DIR}/lib/cmake/opencv4)
    set(Poco_DIR ${PREBUILD_DIR}/lib/cmake/Poco)
    
    # 检查ONNX Runtime
    set(ONNX_ROOT_DIR ${PREBUILD_DIR})
    set(ONNX_INCLUDE_DIRS ${ONNX_ROOT_DIR}/include)

    # 定义基本的ONNX Runtime库
    set(ONNX_LIBRARIES ${PREBUILD_DIR}/lib/libonnxruntime.so)

    # 检查其他可选的依赖库
    foreach(lib_name onnx onnx_proto nsync_cpp cpuinfo)
        if(EXISTS ${ONNX_ROOT_DIR}/lib/lib${lib_name}.so)
            list(APPEND ONNX_LIBRARIES ${ONNX_ROOT_DIR}/lib/lib${lib_name}.so)
        endif()
    endforeach()

    # 验证头文件是否存在
    if(NOT EXISTS ${ONNX_INCLUDE_DIRS}/onnxruntime/onnxruntime_cxx_api.h)
        message(FATAL_ERROR "ONNX Runtime头文件未找到: ${ONNX_INCLUDE_DIRS}/onnxruntime/onnxruntime_cxx_api.h")
    endif()

    # 至少要有基本的ONNX Runtime库
    if(NOT EXISTS ${ONNX_ROOT_DIR}/lib/libonnxruntime.so)
        message(FATAL_ERROR "ONNX Runtime基础库未找到: ${ONNX_ROOT_DIR}/lib/libonnxruntime.so")
    endif()

    message(STATUS "Found ONNX Runtime:")
    message(STATUS "  Include path: ${ONNX_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${ONNX_LIBRARIES}")
endif()

# V4L2后端
option(USE_V4L2 "Enable V4L2 backend" ON)
if(USE_V4L2)
    add_definitions(-DUSE_V4L2)
endif()

# 在find_package(ONNX REQUIRED)之前添加
list(APPEND CMAKE_PREFIX_PATH 
    /usr/local
    ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}
)

# 添加包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${ONNX_INCLUDE_DIRS}
)

# 查找OpenCV
find_package(OpenCV REQUIRED core imgproc imgcodecs)

# 查找ONNX Runtime
find_package(ONNX REQUIRED)

# 查找Poco库
find_package(Poco REQUIRED COMPONENTS Foundation Net JSON)
if(NOT Poco_FOUND)
    message(FATAL_ERROR "Poco库未找到。请确保Poco已正确安装在: ${PREBUILD_DIR}")
endif()

message(STATUS "Found Poco:")
message(STATUS "  Version: ${Poco_VERSION}")
message(STATUS "  Include Dirs: ${Poco_INCLUDE_DIRS}")
message(STATUS "  Libraries: ${Poco_LIBRARIES}")

# 设置Protobuf查找路径
set(Protobuf_ROOT ${PREBUILD_DIR})
set(Protobuf_USE_STATIC_LIBS ON)

# 查找Protobuf
find_package(Protobuf REQUIRED)

# 添加调试信息
message(STATUS "Found Protobuf:")
message(STATUS "  Version: ${Protobuf_VERSION}")
message(STATUS "  Include path: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "  Libraries: ${Protobuf_LIBRARIES}")

# 设置 ONNX Runtime 依赖
set(ONNX_LIBRARIES
    ${PREBUILD_DIR}/lib/libonnxruntime.so
    ${Protobuf_LIBRARIES}
)

# 添加可执行文件
add_executable(video_streaming_app
    src/main.cpp
    src/v4l2_capture.cpp      # V4L2实现
    src/web_server.cpp        # Web服务器
    src/image_processor.cpp   # 图像处理
)

# 设置包含目录
target_include_directories(video_streaming_app
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${ONNX_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${Poco_INCLUDE_DIRS}
)

# 设置rpath
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${PREBUILD_DIR}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# 修改target_link_libraries，确保正确的链接顺序
target_link_libraries(video_streaming_app
    PRIVATE
    ${ONNX_LIBRARIES}      # ONNX 相关库放在前面
    ${Protobuf_LIBRARIES}  # 明确添加 protobuf
    ${OpenCV_LIBS}
    Poco::Foundation
    Poco::Net
    Poco::JSON
    ${CMAKE_THREAD_LIBS_INIT}
)

# 添加链接选项
target_link_options(video_streaming_app
    PRIVATE
    -Wl,--no-as-needed    # 确保所有依赖都被链接
    -Wl,--whole-archive   # 强制链接所有符号
    -Wl,--no-whole-archive
)

# 安装规则
install(TARGETS video_streaming_app
    RUNTIME DESTINATION bin
)

# 添加示例程序
# add_subdirectory(examples)

# 设置ONNX Runtime的CMake包配置路径
set(ONNX_DIR ${PREBUILD_DIR}/lib/cmake/onnxruntime)
