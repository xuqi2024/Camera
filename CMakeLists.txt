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
list(APPEND CMAKE_MODULE_PATH 
    ${CMAKE_SOURCE_DIR}/cmake
    ${CMAKE_SOURCE_DIR}/cmake/modules
)

# 检查预编译库是否存在
if(NOT EXISTS ${PREBUILD_DIR})
    message(STATUS "Prebuild directory not found, will build from source")
    set(BUILD_FROM_SOURCE TRUE)
endif()

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
else()
    # 使用预编译库
    set(OpenCV_DIR ${PREBUILD_DIR}/lib/cmake/opencv4)
    set(Poco_DIR ${PREBUILD_DIR}/lib/cmake/Poco)
    
    find_package(OpenCV REQUIRED COMPONENTS
        core
        imgproc
        imgcodecs
    )
    find_package(Poco REQUIRED Foundation Net JSON)
endif()

# V4L2后端
option(USE_V4L2 "Enable V4L2 backend" ON)
if(USE_V4L2)
    add_definitions(-DUSE_V4L2)
endif()

# 查找ONNX Runtime
find_package(ONNX REQUIRED)

# 添加包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${ONNX_INCLUDE_DIRS}
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

# 链接库
target_link_libraries(video_streaming_app
    PRIVATE
    ${ONNX_LIBRARIES}
    ${OpenCV_LIBS}
    Poco::Foundation
    Poco::Net
    Poco::JSON
    ${CMAKE_THREAD_LIBS_INIT}
)

# 安装规则
install(TARGETS video_streaming_app
    RUNTIME DESTINATION bin
) 