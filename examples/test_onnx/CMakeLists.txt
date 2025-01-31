# 查找OpenCV和ONNX Runtime
find_package(OpenCV REQUIRED core imgproc imgcodecs)

# 添加可执行文件
add_executable(test_onnx test_onnx.cpp)

# 添加包含目录
target_include_directories(test_onnx
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${ONNX_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(test_onnx
    PRIVATE
    ${ONNX_LIBRARIES}
    ${OpenCV_LIBS}
)

# 设置rpath
set_target_properties(test_onnx PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH TRUE
) 