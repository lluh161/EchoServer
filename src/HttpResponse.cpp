#include "HttpResponse.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <cstring>

HttpResponse::HttpResponse() {
    reset();
}

HttpResponse::~HttpResponse() {}

void HttpResponse::reset() {
    statusCode_ = 200;
    headers_.clear();
    body_.clear();
    fileContent_.clear();
}

// 加载静态文件（Day24 核心）
bool HttpResponse::loadFile(const std::string& path) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) return false;

    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return false;
    }

    fileContent_.resize(st.st_size);
    ssize_t n = read(fd, &fileContent_[0], st.st_size);
    close(fd);

    if (n != st.st_size) return false;

    body_ = fileContent_;
    setHeader("Content-Type", getContentType(path));
    return true;
}

// 获取Content-Type（Day24 核心）
std::string HttpResponse::getContentType(const std::string& path) const {
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos) return "text/plain";

    std::string ext = path.substr(dot);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".html") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "application/javascript";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    if (ext == ".ico") return "image/x-icon";
    return "text/plain";
}

// 生成HTTP响应
std::string HttpResponse::toString() const {
    std::string res;
    char buf[1024];

    snprintf(buf, sizeof(buf), "HTTP/1.1 %d OK\r\n", statusCode_);
    res += buf;

    for (auto& p : headers_) {
        snprintf(buf, sizeof(buf), "%s: %s\r\n", p.first.c_str(), p.second.c_str());
        res += buf;
    }

    snprintf(buf, sizeof(buf), "Content-Length: %zu\r\n", body_.size());
    res += buf;
    res += "Connection: close\r\n\r\n";
    res += body_;

    return res;
}