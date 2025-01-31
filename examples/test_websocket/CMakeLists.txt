# 查找Poco库
find_package(Poco REQUIRED Foundation Net)

# 添加可执行文件
add_executable(test_websocket test_websocket.cpp)

# 链接Poco库
target_link_libraries(test_websocket
    PRIVATE
    Poco::Foundation
    Poco::Net
) 