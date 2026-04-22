#pragma once
#include <functional>
#include <cstdint>

// 仅Linux包含epoll头文件并声明Epoll
#if defined(__linux__)
#include <sys/epoll.h>
class Epoll;
#endif

// 回调类型定义
using ReadCallback = std::function<void()>;
using WriteCallback = std::function<void()>;

class Channel
{
public:
    Channel(void* loop, int fd);

    // 禁止拷贝，允许移动
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;
    Channel(Channel&&) = default;
    Channel& operator=(Channel&&) = default;

    // 读事件接口
    void handleRead();
    void setReadCallback(ReadCallback cb);
    void enableReading();

    // 写事件接口
    void handleWrite();
    void setWriteCallback(WriteCallback cb);
    void enableWriting();
    void disableWriting();

    // 属性接口
    int getFd() const { return fd_; }
    uint32_t getEvents() const { return events_; }
    void setRevents(uint32_t revt) { revents_ = revt; }
    bool isInEpoll() const { return inEpoll_; }
    void setInEpoll(bool in) { inEpoll_ = in; }

private:
    void updateEvents();

    void* loop_;
    int fd_;
    uint32_t events_;
    uint32_t revents_;
    bool inEpoll_;

    ReadCallback readCallback;
    WriteCallback writeCallback;
};