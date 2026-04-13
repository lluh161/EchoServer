#include "EchoServer.h"
#include "Channel.h"
#include "Buffer.h"
#include "HttpRequest.h"
#include "Log.h"
#include "HttpResponse.h"
#include "InetAddress.h"
#include <iostream>
#include <functional>
#include <unistd.h>
#include <thread>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


EchoServer::EchoServer(EventLoop* loop, const InetAddress& listenAddr)//初始化服务器
    : loop_(loop),//主Reactor
      socket_(std::make_unique<Socket>()),
      acceptChannel_(std::make_unique<Channel>(loop,socket_->fd())),
      threadPool_(std::make_unique<ThreadPool>(4))
{
    (void)loop_;
    //设置端口复用
    socket_->setReuseAddr();
    socket_->setReusePort();

    socket_->bind(listenAddr);//绑定IP+端口
    socket_->listen();//开始监听

    //新连接事件绑定到handleNewConnection
    acceptChannel_->setReadCallback(std::bind(&EchoServer::handleNewConnection,this));
    acceptChannel_->enableReading();//注册到epoll
    loop_->updateChannel(acceptChannel_.get());

    //初始化业务线程池
    threadPool_=std::make_unique<ThreadPool>(4);
    threadPool_->start();

    //创建3个从Reactor，处理客户端IO
    for(int i=0;i<3;++i){
        subLoops_.push_back(std::make_unique<EventLoop>());
    }
}

void EchoServer::start() {
    for (auto& subLoop : subLoops_) {
        std::thread([loop = subLoop.get()]() {
            loop->loop();
        }).detach();
    }
}

void EchoServer::handleNewConnection() {
    //接受新连接
    int cfd = accept(socket_->fd(), NULL, NULL);
    if (cfd < 0) {
        LOGE("accept 失败");
        return;
    }

    LOGI("新客户端连接 fd=%d", cfd);

    //你框架已有的非阻塞设置
    int flags = fcntl(cfd, F_GETFL, 0);
    fcntl(cfd, F_SETFL, flags | O_NONBLOCK);

    //轮询选择从Reactor
    static int next = 0;
    EventLoop* subLoop = subLoops_[next++ % subLoops_.size()].get();

    //创建客户端channel
    Channel* clientChannel = new Channel(subLoop, cfd);

    // 不改变 ReadCallback 类型！
    clientChannel->setReadCallback([this, cfd]() {
        Buffer* buf = new Buffer();
        handleMessage(cfd, buf);
    });

    //注册读事件
    clientChannel->enableReading();
    subLoop->updateChannel(clientChannel);
}

void EchoServer::handleMessage(int cfd, Buffer* buf){
    LOGD("开始处理客户端 fd=%d", cfd);

    //读取请求
    char buffer[4096];
    ssize_t n = read(cfd, buffer, sizeof(buffer));
    if (n <= 0) {
        LOGW("客户端关闭连接 fd=%d", cfd);
        close(cfd);
        delete buf;
        return;
    }

    //解析HTTP
    HttpRequest req;
    req.parse(buffer, n);

    //静态文件服务
    HttpResponse resp;

    std::string path = req.path();
    LOGI("请求路径：%s", path.c_str());

    if (path == "/") path = "/index.html";
    std::string filePath = "/Users/hh/Desktop/EchoServer/www" + path;

    if (!resp.loadFile(filePath)) {
        LOGW("文件不存在：%s", filePath.c_str());
        resp.setStatusCode(404);
        resp.setBody("<h1>404 Not Found</h1>");
        resp.setHeader("Content-Type", "text/html");
    }else{
        LOGD("成功读取文件：%s", filePath.c_str());
    }

    //发送响应
    std::string response = resp.toString();
    send(cfd, response.data(), response.size(), MSG_NOSIGNAL);

    //优雅关闭
    shutdown(cfd, SHUT_WR);
    close(cfd);

    delete buf;
    LOGD("响应完成，关闭 fd=%d", cfd);
}