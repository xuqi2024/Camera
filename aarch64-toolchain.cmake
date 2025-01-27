set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# 指定交叉编译器
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# 搜索程序时只在目标目录下搜索
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# 搜索库时只在目标目录下搜索
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# 搜索头文件时只在目标目录下搜索
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY) 