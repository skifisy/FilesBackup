# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(backup VERSION 1.0)

#复制测试的test.jpg
file(COPY ${CMAKE_SOURCE_DIR}/BackupServer/test.jpg DESTINATION ${CMAKE_BINARY_DIR}/BackupServer)

# 将 config.json 从源目录复制到构建目录
file(COPY ${CMAKE_SOURCE_DIR}/config.json DESTINATION ${CMAKE_BINARY_DIR}/BackupGUI)
file(COPY ${CMAKE_SOURCE_DIR}/config.json DESTINATION ${CMAKE_BINARY_DIR}/BackupServer)
add_subdirectory(BackupServer)
add_subdirectory(BackupGUI)

# 启用测试
enable_testing()