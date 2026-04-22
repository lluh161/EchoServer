#include "EchoServer.h"
#include "Channel.h"
#include "Buffer.h"
#include "HttpRequest.h"
#include "Log.h"
#include "User.h"
#include "HttpResponse.h"
#include "InetAddress.h"
#include "ConnectionPool.h"
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
#include <memory>
#include <stdlib.h>
#include <algorithm>


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

    //已有的非阻塞设置
    int flags = fcntl(cfd, F_GETFL, 0);
    fcntl(cfd, F_SETFL, flags | O_NONBLOCK);

    //轮询选择从Reactor
    static int next = 0;
    EventLoop* subLoop = subLoops_[next++ % subLoops_.size()].get();

    //连接建立 10 秒后关闭，不刷新、不传递、不报错
    subLoop->runAfter(10000, [=]() {
        LOGW("连接超时关闭 fd=%d", cfd);
        close(cfd);
    });
    
    //创建客户端channel
    Channel* clientChannel = new Channel(subLoop, cfd);

    //创建读Buffer
    clientBuffers_.push_back(std::make_unique<Buffer>());
    Buffer* readBuf = clientBuffers_.back().get();
    //创建写Buffer
    clientWriteBuffers_.push_back(std::make_unique<Buffer>());
    Buffer* writeBuf = clientWriteBuffers_.back().get();


    //收到数据就调用handleMessage，把永久绑定的Buffer传进去
    clientChannel->setReadCallback(std::bind(&EchoServer::handleMessage, this, cfd, readBuf));

    //开启读事件监听（ET边缘触发）
    clientChannel->enableReading();

    //注册读事件
    clientChannel->enableReading();
    subLoop->updateChannel(clientChannel);
}

// 解析POST参数
std::string getParam(const std::string& body, const std::string& key) {
    size_t pos = body.find(key + "=");
    if (pos == std::string::npos) return "";

    pos += key.size() + 1;
    size_t end = body.find('&', pos);
    if (end == std::string::npos) end = body.size();

    return body.substr(pos, end - pos);
}

void EchoServer::handleMessage(int cfd, Buffer* buf) {
    LOGD("开始处理客户端 fd=%d", cfd);

    //读取请求
    ssize_t n = buf->readFd(cfd);
    if(n<=0){
        LOGW("客户端关闭连接 fd=%d", cfd);
        close(cfd);
        return;
    }

    // 解析HTTP
    HttpRequest req;
    req.parse(buf->peek(), buf->readableBytes());

    buf->retrieve(n);

    HttpResponse resp;
    std::string path = req.path();
    LOGI("请求路径：%s", path.c_str());

    User service;
    //注册 API
    if (path == "/api/register" && req.method() == "POST") {
        // 变量名：user, pwd
        std::string user = getParam(req.body(), "username");
        std::string pwd = getParam(req.body(), "password");
        bool ok = service.reg(user, pwd);

        resp.setStatusCode(200);
        resp.setHeader("Content-Type", "application/json");
        if (ok) {
            resp.setBody(R"({"code":0,"msg":"注册成功"})");
        } else {
            resp.setBody(R"({"code":-1,"msg":"注册失败，用户名已存在"})");
        }
    }
    //登录 API
    else if (path == "/api/login" && req.method() == "POST") {
        std::string user = getParam(req.body(), "username");
        std::string pwd = getParam(req.body(), "password");
        bool ok = service.login(user, pwd);

        resp.setStatusCode(200);
        resp.setHeader("Content-Type", "application/json");
        if (ok) {
            resp.setBody(R"({"code":0,"msg":"登录成功"})");
        } else {
            resp.setBody(R"({"code":-1,"msg":"用户名或密码错误"})");
        }
    }
    //静态文件
    else {
        if (path == "/") path = "/index.html";
        std::string filePath = "/Users/hh/Desktop/EchoServer/www" + path;

        if (!resp.loadFile(filePath)) {
            LOGW("文件不存在：%s", filePath.c_str());
            resp.setStatusCode(404);
            resp.setBody("<h1>404 Not Found</h1>");
            resp.setHeader("Content-Type", "text/html");
        } else {
            LOGD("成功读取文件：%s", filePath.c_str());
        }
    }

    //双平台完美兼容
    #if defined(__linux__)
    //Linux：保留异步Buffer+Channel+Epoll高性能逻辑！一行不动！
    Buffer* writeBuf = clientWriteBuffers_.back().get();
    writeBuf->append(resp.toString().data(), resp.toString().size());
    Channel* clientChannel = clientChannels_.back().get();
    clientChannel->setWriteCallback(std::bind(&EchoServer::handleWrite, this, clientChannel, writeBuf));
    clientChannel->enableWriting();
    #else
    // Mac：同步
    std::string response = resp.toString();
    send(cfd, response.data(), response.size(), MSG_NOSIGNAL);
    shutdown(cfd, SHUT_WR);
    close(cfd);
    #endif
}

//写事件回调
void EchoServer::handleWrite(Channel* channel, Buffer* writeBuf) {
    // 从Buffer向fd发送数据，非阻塞发送
    ssize_t n = writeBuf->writeFd(channel->getFd());
    if(n > 0) {
        // 数据全部发送完毕
        if(writeBuf->readableBytes() == 0) {
            //取消Epoll可写事件监听
            channel->disableWriting();
            //优雅关闭写端、释放资源、关闭fd
            shutdown(channel->getFd(), SHUT_WR);
            LOGD("响应全部发送完成, 关闭fd=%d", channel->getFd());
            close(channel->getFd());
        }
    } else if(errno != EAGAIN && errno != EWOULDBLOCK) {
        //发送出错，直接关闭连接
        LOGD("fd=%d 发送错误，关闭连接", channel->getFd());
        close(channel->getFd());
    }
}