# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(backup VERSION 1.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# cmake -DCMAKE_BUILD_TYPE=Debug ..
set(CMAKE_CXX_FLAGS_DEBUG "-g")         # 在 Debug 模式下生成调试信息

# 添加包含目录 (头文件所在目录)
include_directories(${PROJECT_SOURCE_DIR}/include)

# 查找 Crypto++ 库
# find_package(CryptoPP REQUIRED)
find_library(CRYPTOPP_LIBRARY NAMES cryptopp)

# 查找 src 目录下的所有 cpp 文件和 c 文件并存入变量 SRC_FILES
file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)


########  配置单元测试  ###########

# 查找 Google Test 库
find_package(GTest REQUIRED)

# 启用测试
enable_testing()

# 查找测试代码的源文件
file(GLOB_RECURSE TEST_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

# 添加测试代码的源文件
add_executable(unit_tests ${TEST_FILES})

list(REMOVE_ITEM SRC_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)
add_library(backuplib ${SRC_FILES})
# target_link_libraries(backuplib CryptoPP::CryptoPP) 
target_link_libraries(backuplib ${CRYPTOPP_LIBRARY}) 

# 生成可执行文件
add_executable(backup ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(backup backuplib)


# 链接 GTest 库
target_link_libraries(unit_tests GTest::GTest GTest::Main backuplib)

# 添加测试
add_test(NAME unit_tests COMMAND unit_tests)
