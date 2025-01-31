# FindOpenCV.cmake

# 添加调试信息
message(STATUS "Looking for OpenCV in: ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}")

# 查找OpenCV配置文件
find_path(OpenCV_DIR
    NAMES OpenCVConfig.cmake
    PATHS
        ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}/lib/cmake/opencv4
        ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}/share/opencv4
    NO_DEFAULT_PATH
)

if(OpenCV_DIR)
    message(STATUS "Found OpenCVConfig.cmake in: ${OpenCV_DIR}")
    include(${OpenCV_DIR}/OpenCVConfig.cmake)
    
    # 验证找到的OpenCV版本
    if(OpenCV_FOUND)
        message(STATUS "Found OpenCV ${OpenCV_VERSION}")
        message(STATUS "  Include path: ${OpenCV_INCLUDE_DIRS}")
        message(STATUS "  Libraries: ${OpenCV_LIBS}")
        return()
    endif()
endif()

# 如果没有找到配置文件，尝试手动查找组件
message(STATUS "OpenCVConfig.cmake not found, trying manual component search...")

# 查找头文件
find_path(OpenCV_INCLUDE_DIR
    NAMES opencv2/core.hpp
    PATHS
        ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}/include
        ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}/include/opencv4
    NO_DEFAULT_PATH
)

# 查找各个模块的库文件
set(OpenCV_COMPONENTS core imgproc imgcodecs videoio)
set(OpenCV_FOUND TRUE)
set(OpenCV_LIBS "")

foreach(comp ${OpenCV_COMPONENTS})
    find_library(OpenCV_${comp}_LIBRARY
        NAMES opencv_${comp}
        PATHS
            ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}/lib
            ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}/lib64
        NO_DEFAULT_PATH
    )
    
    if(OpenCV_${comp}_LIBRARY)
        message(STATUS "Found OpenCV component ${comp}: ${OpenCV_${comp}_LIBRARY}")
        list(APPEND OpenCV_LIBS ${OpenCV_${comp}_LIBRARY})
    else()
        message(STATUS "Could not find OpenCV component: ${comp}")
        set(OpenCV_FOUND FALSE)
    endif()
endforeach()

if(OpenCV_INCLUDE_DIR AND OpenCV_FOUND)
    set(OpenCV_INCLUDE_DIRS ${OpenCV_INCLUDE_DIR})
    
    # 获取版本信息
    if(EXISTS "${OpenCV_INCLUDE_DIR}/opencv2/core/version.hpp")
        file(STRINGS "${OpenCV_INCLUDE_DIR}/opencv2/core/version.hpp" opencv_version_str
             REGEX "^#define CV_VERSION_[A-Z]+[ ]+ [0-9]+$")
        string(REGEX REPLACE ".*CV_VERSION_MAJOR[ ]+([0-9]+).*" "\\1" OpenCV_VERSION_MAJOR "${opencv_version_str}")
        string(REGEX REPLACE ".*CV_VERSION_MINOR[ ]+([0-9]+).*" "\\1" OpenCV_VERSION_MINOR "${opencv_version_str}")
        string(REGEX REPLACE ".*CV_VERSION_REVISION[ ]+([0-9]+).*" "\\1" OpenCV_VERSION_PATCH "${opencv_version_str}")
        set(OpenCV_VERSION "${OpenCV_VERSION_MAJOR}.${OpenCV_VERSION_MINOR}.${OpenCV_VERSION_PATCH}")
    endif()
    
    message(STATUS "Found OpenCV ${OpenCV_VERSION}")
    message(STATUS "  Include path: ${OpenCV_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${OpenCV_LIBS}")
else()
    message(FATAL_ERROR "Could not find OpenCV in ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}")
endif()

mark_as_advanced(
    OpenCV_INCLUDE_DIR
    OpenCV_DIR
    ${OpenCV_COMPONENTS}
) 