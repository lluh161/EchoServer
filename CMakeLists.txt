cmake_minimum_required(VERSION 3.16)
project(EchoServer)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# M3 芯片 MySQL 正确路径（Homebrew 标准安装）
if(NOT APPLE)
    message(FATAL_ERROR "本项目仅支持 MacOS (M1/M3)")
endif()

# 把 include 目录加入头文件搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/include  # 头文件都在 include 目录下
    /opt/homebrew/opt/mysql-client/include  # MySQL 头文件路径
)

# MySQL 路径（固定正确）
set(MYSQL_ROOT /opt/homebrew/opt/mysql-client)
include_directories(${MYSQL_ROOT}/include)
link_directories(${MYSQL_ROOT}/lib)

# 查找 MySQL 库（只调用一次，避免重复）
find_library(MYSQL_LIB NAMES mysqlclient PATHS ${MYSQL_ROOT}/lib REQUIRED)

# 收集所有源文件
file(GLOB_RECURSE SRC_FILES 
    src/*.cpp
    src/**/*.cpp
)

# 生成可执行文件
add_executable(echo_server ${SRC_FILES})

# 统一签名：全部使用 PRIVATE
target_link_libraries(echo_server
    PRIVATE
    ${MYSQL_LIB}
    pthread
)