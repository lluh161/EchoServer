# 编译指南

## Linux/Mac 一键编译
mkdir build
cd build
cmake ..
make -j4

## 输出位置
build/bin/echo_server

## 交叉编译 ARM
cmake .. -DCMAKE_TOOLCHAIN_FILE=arm64.toolchain
make -j4