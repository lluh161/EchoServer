cmake_minimum_required(VERSION 3.16)
project(EchoServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")  # 【新增：编译警告】

# 交叉编译 Linux
if (DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
endif()

# 公共代码
include_directories(include)
file(GLOB_RECURSE SRC_FILES
    src/*.cpp
    src/*/*.cpp
)

add_executable(echo_server ${SRC_FILES})

# pthread 跨平台统一加
find_package(Threads REQUIRED)
target_link_libraries(echo_server PRIVATE Threads::Threads)

# Mac 环境自动加 MySQL
if(APPLE AND NOT CMAKE_CROSSCOMPILING)
    set(MYSQL_ROOT /opt/homebrew/opt/mysql-client)
    include_directories(${MYSQL_ROOT}/include)
    link_directories(${MYSQL_ROOT}/lib)
    find_library(MYSQL_LIB NAMES mysqlclient PATHS ${MYSQL_ROOT}/lib REQUIRED)
    target_link_libraries(echo_server PRIVATE ${MYSQL_LIB})
endif()

# 输出目录统一，方便部署
set_target_properties(echo_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)