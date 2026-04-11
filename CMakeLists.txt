cmake_minimum_required(VERSION 3.16)
project(EchoServer)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 头文件搜索路径：指向 include 目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 自动搜索 src 目录下所有 .cpp 源文件
file(GLOB_RECURSE ECHO_SERVER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# 生成可执行文件
add_executable(echo_server ${ECHO_SERVER_SOURCES})

# 链接线程库（必须，Epoll/线程池依赖）
target_link_libraries(echo_server pthread)