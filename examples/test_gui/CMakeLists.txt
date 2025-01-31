# 查找Qt和OpenCV
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(OpenCV REQUIRED core imgproc imgcodecs videoio)

# 添加可执行文件
add_executable(test_gui test_gui.cpp)

# 设置Qt自动处理
set_target_properties(test_gui PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# 链接库
target_link_libraries(test_gui
    PRIVATE
    Qt5::Core
    Qt5::Widgets
    ${OpenCV_LIBS}
) 