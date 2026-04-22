#include "Channel.h"

// Linux平台
#if defined(__linux__)
#include "Epoll.h"
#else
// Mac平台：在这里定义，只在本文件有效，不会重复定义
#define EPOLLIN  0x0001
#define EPOLLOUT 0x0004
#endif

Channel::Channel(void* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), inEpoll_(false)
{}

void Channel::updateEvents()
{
#if defined(__linux__)
    Epoll* epoll = (Epoll*)loop_;
    epoll->updateChannel(this);
#endif
}

void Channel::enableReading()
{
    events_ |= EPOLLIN;
    updateEvents();
}

void Channel::enableWriting()
{
    events_ |= EPOLLOUT;
    updateEvents();

    // Mac专用：立即触发写回调，让响应发出去
#if defined(__APPLE__)
    if(writeCallback) writeCallback();
#endif
}

void Channel::disableWriting()
{
    events_ &= ~EPOLLOUT;
    updateEvents();
}

void Channel::handleRead()
{
    if(readCallback) readCallback();
}

void Channel::handleWrite()
{
    if(writeCallback) writeCallback();
}

void Channel::setReadCallback(ReadCallback cb)
{
    readCallback = std::move(cb);
}

void Channel::setWriteCallback(WriteCallback cb)
{
    writeCallback = std::move(cb);
}