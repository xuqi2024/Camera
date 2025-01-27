cmake_minimum_required(VERSION 3.10)
project(video_streaming_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找必要的包
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost 1.64 REQUIRED COMPONENTS system date_time)

# 添加 Crow 依赖（header-only 库）
include(FetchContent)
FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow
    GIT_TAG v1.0+5
)
FetchContent_MakeAvailable(Crow)

# 包含目录
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)

# 添加可执行文件
add_executable(video_streaming_app
    src/main.cpp
    src/video_capture.cpp
    src/web_server.cpp
)

# 链接库
target_link_libraries(video_streaming_app
    PRIVATE
    ${OpenCV_LIBS}
    Crow::Crow
    Threads::Threads
    Boost::system
    Boost::date_time
) 