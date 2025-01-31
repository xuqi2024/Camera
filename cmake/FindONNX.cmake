# FindONNX.cmake
# 查找ONNX Runtime库和头文件

# 添加调试信息
message(STATUS "Looking for ONNX Runtime in: ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}")

# 使用预定义的变量
if(DEFINED ONNX_INCLUDE_DIRS AND DEFINED ONNX_LIBRARIES)
    set(ONNX_FOUND TRUE)
else()
    # 查找头文件
    find_path(ONNX_INCLUDE_DIRS
        NAMES onnxruntime/core/session/onnxruntime_cxx_api.h
        PATHS
            ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}/include
            ${ONNX_ROOT_DIR}/include
        NO_DEFAULT_PATH
    )

    # 查找库文件
    find_library(ONNX_LIBRARIES
        NAMES onnxruntime
        PATHS
            ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}/lib
            ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}/lib64
            ${ONNX_ROOT_DIR}/lib
        NO_DEFAULT_PATH
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ONNX
    REQUIRED_VARS 
        ONNX_LIBRARIES 
        ONNX_INCLUDE_DIRS
)

if(ONNX_FOUND)
    # 创建导入目标
    if(NOT TARGET ONNX::ONNX)
        add_library(ONNX::ONNX UNKNOWN IMPORTED)
        set_target_properties(ONNX::ONNX PROPERTIES
            IMPORTED_LOCATION "${ONNX_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${ONNX_INCLUDE_DIRS}"
        )
    endif()

    message(STATUS "Found ONNX Runtime:")
    message(STATUS "  Include path: ${ONNX_INCLUDE_DIRS}")
    message(STATUS "  Library: ${ONNX_LIBRARIES}")
else()
    message(FATAL_ERROR "Could not find ONNX Runtime in ${CMAKE_SOURCE_DIR}/prebuild/${PLATFORM_NAME}")
endif()

mark_as_advanced(ONNX_INCLUDE_DIRS ONNX_LIBRARIES) 