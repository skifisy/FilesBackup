# 查找OpenSSL的加密库
find_library(CRYPTO_LIBRARY NAMES crypto)

# 查找 src 目录下的所有 cpp 文件和 c 文件并存入变量 SRC_FILES
file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)

# 生成库文件
list(REMOVE_ITEM SRC_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)
add_library(backuplib ${SRC_FILES})
target_link_libraries(backuplib ${CRYPTO_LIBRARY})

# 生成可执行文件
# add_executable(backup ${PROJECT_SOURCE_DIR}/src/main.cpp)
# target_link_libraries(backup backuplib)
