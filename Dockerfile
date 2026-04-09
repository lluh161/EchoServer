# 阶段1：用 Linux 环境编译你的代码（Epoll 只在 Linux 可用）
FROM gcc:14-bookworm AS builder

WORKDIR /app

# 把整个项目复制进容器
COPY . .

# 编译（适配你的目录结构，Linux 环境支持 Epoll）
RUN g++ -std=c++17 \
    -I./项目搭建/网络框架/include \
    项目搭建/网络框架/src/main.cpp \
    项目搭建/网络框架/src/EchoServer.cpp \
    项目搭建/网络框架/src/EventLoop.cpp \
    项目搭建/网络框架/src/ThreadPool.cpp \
    项目搭建/网络框架/src/InetAddress.cpp \
    项目搭建/网络框架/src/Socket.cpp \
    项目搭建/网络框架/src/Epoll.cpp \
    项目搭建/网络框架/src/Channel.cpp \
    项目搭建/网络框架/src/Buffer.cpp \
    -o echo_server -pthread -Wno-deprecated

# 阶段2：用轻量 Debian 镜像运行
FROM debian:bookworm-slim

WORKDIR /app

# 从编译阶段复制可执行文件
COPY --from=builder /app/echo_server .

# 暴露 8888 端口
EXPOSE 8888

# 启动服务
CMD ["./echo_server"]