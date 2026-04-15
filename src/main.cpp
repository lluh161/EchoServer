#include "EventLoop.h"
#include "ConnectionPool.h"
#include "InetAddress.h"
#include "EchoServer.h"
#include "Log.h"

int main(){
    LOGI("初始化 MySQL 连接池...");
    //初始化连接池
    ConnectionPool::instance().init(
        "127.0.0.1",
        "root",         // 你的 MySQL 用户名
        "123456",       // 你的 MySQL 密码
        "testdb",       // 数据库名
        3306,           // 端口
        10              // 最大连接数
    );

    EventLoop mainLoop;//主事件循环
    InetAddress addr(8888);//监听8888
    EchoServer server(&mainLoop, addr);//创建Echo服务器

    
    LOGI("服务器启动，监听端口 8888...");
    server.start();//启动服务器
    mainLoop.loop();//启动主循环

    //服务器关闭时，释放连接池
    ConnectionPool::instance().close();
    LOGI("服务器关闭");
    return 0;
}
