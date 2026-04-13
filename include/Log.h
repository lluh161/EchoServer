#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string>

// 日志级别
#define LOG_DEBUG    0
#define LOG_INFO     1
#define LOG_WARN     2
#define LOG_ERROR    3

// 默认日志级别
#define LOG_LEVEL    LOG_DEBUG

// 对外接口
#define LOGD(...) logPrint(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOGI(...) logPrint(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define LOGW(...) logPrint(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOGE(...) logPrint(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)

// 核心打印函数
void logPrint(int level, const char* file, int line, const char* fmt, ...);

#endif