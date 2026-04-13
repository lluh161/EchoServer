#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string>
#include <unordered_map>
#include <fstream>

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    // 设置状态码
    void setStatusCode(int code) { statusCode_ = code; }
    // 设置响应头
    void setHeader(const std::string& key, const std::string& value) { headers_[key] = value; }
    // 设置响应体
    void setBody(const std::string& body) { body_ = body; }
    // 加载静态文件
    bool loadFile(const std::string& path);

    // 生成完整HTTP响应字符串
    std::string toString() const;

    // 重置响应
    void reset();

private:
    // 根据文件后缀获取Content-Type
    std::string getContentType(const std::string& path) const;

    int statusCode_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
    std::string fileContent_;
};

#endif // HTTPRESPONSE_H