# 查找OpenCV
find_package(OpenCV REQUIRED core imgproc imgcodecs)

# 添加可执行文件
add_executable(test_v4l2 test_v4l2.cpp)

# 链接库
target_link_libraries(test_v4l2
    PRIVATE
    ${OpenCV_LIBS}
) 