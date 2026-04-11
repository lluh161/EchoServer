#include <iostream>
#include "Log.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>

std::string Log::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time);
    
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void Log::log(const std::string& msg) {
    std::string time = getCurrentTime();
    std::string logMsg = "[" + time + "] " + msg;
    
    // 控制台输出
    std::cout << logMsg << std::endl;
    
    // 可选：写入日志文件
    std::ofstream file("server.log", std::ios::app);
    if (file.is_open()) {
        file << logMsg << std::endl;
        file.close();
    } else {
        std::cerr << "日志文件打开失败！" << std::endl;
    }
}