# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(BackupServer VERSION 1.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# 设置全局 C++ 编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -O2")

# cmake -DCMAKE_BUILD_TYPE=Debug ..
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG -fsanitize=address,leak,undefined")         # 在 Debug 模式下生成调试信息

# 添加包含目录 (头文件所在目录)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(src)

# 启用测试
enable_testing()
add_subdirectory(test)
# 添加测试
add_test(NAME unit_tests COMMAND ./test/unit_tests)