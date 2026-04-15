#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <mysql/mysql.h>
#include <queue>
#include <mutex>
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <chrono>

// RAII 连接守卫：离开作用域自动归还连接
class ConnectionGuard {
public:
    using ReleaseFunc = std::function<void(MYSQL*)>;

    // 构造：传入连接和归还函数
    ConnectionGuard(MYSQL* conn, ReleaseFunc release)
        : conn_(conn), release_(release) {}

    // 析构：自动归还连接到池
    ~ConnectionGuard() {
        if (conn_ && release_)
            release_(conn_);
    }

    // 禁止拷贝
    ConnectionGuard(const ConnectionGuard&) = delete;
    ConnectionGuard& operator=(const ConnectionGuard&) = delete;

    // 获取原生 MYSQL*
    MYSQL* get() { return conn_; }

private:
    MYSQL* conn_;
    ReleaseFunc release_;
};

// 单例 MySQL 连接池
class ConnectionPool {
public:
    // 获取单例实例
    static ConnectionPool& instance() {
        static ConnectionPool inst;
        return inst;
    }

    // 初始化连接池：创建 max_conn 个连接
    bool init(const std::string& host, const std::string& user,
              const std::string& passwd, const std::string& db,
              int port, int max_conn);

    // 从池获取一个连接（返回 RAII 守卫）
    std::unique_ptr<ConnectionGuard> getConn();

    // 关闭所有连接
    void close();

    ~ConnectionPool() { close(); }

private:
    ConnectionPool() = default;

    // 创建一条新 MySQL 连接
    MYSQL* createConn();

    std::queue<MYSQL*> pool_;
    std::mutex mtx_;
    std::string host_, user_, passwd_, db_;
    int port_;
    bool closed_ = false;
};

#endif