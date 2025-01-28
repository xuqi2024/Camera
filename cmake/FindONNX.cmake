# FindONNX.cmake
# 查找ONNX Runtime库和头文件

# 查找头文件
find_path(ONNX_INCLUDE_DIRS
    NAMES onnxruntime_cxx_api.h
    PATHS
        /usr/local/include
        /usr/include
        /usr/local/include/onnxruntime
        /usr/include/onnxruntime
    PATH_SUFFIXES onnxruntime
)

# 查找库文件
find_library(ONNX_LIBRARIES
    NAMES onnxruntime
    PATHS
        /usr/local/lib
        /usr/lib
        /usr/local/lib64
        /usr/lib64
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ONNX
    REQUIRED_VARS ONNX_LIBRARIES ONNX_INCLUDE_DIRS
)

mark_as_advanced(ONNX_INCLUDE_DIRS ONNX_LIBRARIES) 