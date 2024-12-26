
# 查找 Google Test 库
find_package(GTest REQUIRED)

# 查找测试代码的源文件
file(GLOB_RECURSE TEST_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

# 添加测试代码的源文件
add_executable(unit_tests ${TEST_FILES})

# 链接 GTest 库
target_link_libraries(unit_tests GTest::GTest GTest::Main backuplib)