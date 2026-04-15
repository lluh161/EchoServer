#include "ConnectionPool.h"

// 创建一条 MySQL 连接
MYSQL* ConnectionPool::createConn() {
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) return nullptr;

    // 连接数据库
    if (!mysql_real_connect(conn, host_.c_str(), user_.c_str(),
                            passwd_.c_str(), db_.c_str(), port_, nullptr, 0)) {
        mysql_close(conn);
        return nullptr;
    }
    return conn;
}

// 初始化连接池，预先建立多条连接
bool ConnectionPool::init(const std::string& host, const std::string& user,
                          const std::string& passwd, const std::string& db,
                          int port, int max_conn) {
    // 入参合法性校验
    if (max_conn <= 0) {
        return false;
    }
    if (host.empty() || user.empty() || db.empty()) {
        return false;
    }

    // 加锁保护初始化
    std::lock_guard<std::mutex> lock(mtx_);

    // 保存数据库配置
    host_ = host;
    user_ = user;
    passwd_ = passwd;
    db_ = db;
    port_ = port;

    // 批量预创建连接，带失败处理
    for (int i = 0; i < max_conn; ++i) {
        if (auto c = createConn()) {
            pool_.push(c);
        } 
    }

    // 校验最终连接池状态
    if (pool_.empty()) {
        return false;
    }

    return true;
}

// 获取连接：自动等待，返回 RAII 守卫
std::unique_ptr<ConnectionGuard> ConnectionPool::getConn() {
    std::unique_lock<std::mutex> lock(mtx_);

    // 池空则等待
    while (pool_.empty()) {
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        lock.lock();
    }

    MYSQL* c = pool_.front();
    pool_.pop();

    // 连接失效则重建
    if (mysql_ping(c) != 0) {
        mysql_close(c);
        c = createConn();
    }

    // 返回守卫，析构时自动归还
    return std::make_unique<ConnectionGuard>(c, [this](MYSQL* conn) {
        std::lock_guard<std::mutex> lock(this->mtx_);
        if (!this->closed_) {
            this->pool_.push(conn);
        } else {
            mysql_close(conn);
        }
    });
}

// 关闭池并释放所有连接
void ConnectionPool::close() {
    std::lock_guard<std::mutex> lock(mtx_);
    closed_ = true;
    while (!pool_.empty()) {
        mysql_close(pool_.front());
        pool_.pop();
    }
}