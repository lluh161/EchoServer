FROM ubuntu:22.04
WORKDIR /app
COPY . .

# 容器内自动安装编译环境
RUN apt update && apt install -y g++ cmake make
RUN mkdir build && cd build && cmake .. && make

EXPOSE 8888
CMD ["./build/echo_server"]
