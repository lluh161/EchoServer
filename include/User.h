#ifndef USER_DAO_H
#define USER_DAO_H

#include <string>

// 用户数据库操作类
class User {
public:
    // 登录验证：查用户名密码
    static bool login(const std::string& username, const std::string& password);

    // 注册：插入用户
    static bool reg(const std::string& username, const std::string& password);
};

#endif