#include "User.h"
#include "Log.h"
#include "ConnectionPool.h"

// 用户登录验证
bool User::login(const std::string& username, const std::string& password) {
    // 获取连接（自动归还）
    auto guard = ConnectionPool::instance().getConn();
    if (!guard) return false;
    MYSQL* conn = guard->get();

     char sql[1024];
    std::snprintf(sql, sizeof(sql), 
        "SELECT 1 FROM user WHERE username='%s' AND password='%s'",
        username.c_str(), password.c_str());

    // 执行查询
    if (mysql_query(conn, sql)) return false;

    // 获取结果集
    MYSQL_RES* res = mysql_store_result(conn);
    bool ok = res && mysql_num_rows(res) > 0;
    if (res) mysql_free_result(res);

    return ok;
}

// 用户注册
bool User::reg(const std::string& username, const std::string& password) {
    auto guard = ConnectionPool::instance().getConn();
    if (!guard) {
        LOGW("获取数据库连接失败");
        return false;
    }
    MYSQL* conn = guard->get();

    char sql[1024];
    std::snprintf(sql, sizeof(sql), 
        "INSERT INTO user(username, password) VALUES ('%s', '%s')",
        username.c_str(), password.c_str());

    return mysql_query(conn, sql) == 0;
}