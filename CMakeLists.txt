# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称和版本
project(backup VERSION 1.0)


add_subdirectory(BackupServer)
add_subdirectory(BackupGUI)

# 启用测试
enable_testing()