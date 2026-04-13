#include "Log.h"

static const char* levelStr[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};

void logPrint(int level, const char* file, int line, const char* fmt, ...)
{
    if (level < LOG_LEVEL) return;

    // 时间
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", t);

    // 提取文件名（不要全路径）
    const char* filename = strrchr(file, '/');
    if (filename) filename++;
    else filename = file;

    // 头部
    printf("[%s] [%s] %s:%d | ", 
           timeStr, levelStr[level], filename, line);

    // 内容
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    printf("\n");
    fflush(stdout);
}