# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(backup VERSION 1.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加包含目录 (头文件所在目录)
include_directories(${PROJECT_SOURCE_DIR}/include)

# 查找 src 目录下的所有 cpp 文件并存入变量 SRC_FILES
file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cc)

# 生成可执行文件
add_executable(backup ${SRC_FILES})

# 可选：如果有外部库，可以在这里链接
# target_link_libraries(MyProject some_library)
