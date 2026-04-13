#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <string>
#include <unordered_map>

enum HttpRequestParseState {
    PARSE_REQUEST_LINE,
    PARSE_HEADER,
    PARSE_BODY,
    PARSE_DONE,
    PARSE_ERROR
};

class HttpRequest {
public:
    HttpRequest();
    void reset();
    bool parse(const char* data, int len);

    std::string method() const { return method_; }
    std::string path() const { return path_; }
    std::string version() const { return version_; }
    std::unordered_map<std::string, std::string> headers() const { return headers_; }
    std::string body() const { return body_; }

private:
    bool parseRequestLine(const std::string& line);
    bool parseHeaderLine(const std::string& line);
    int getContentLength() const;

    HttpRequestParseState state_;
    std::string method_;
    std::string path_;
    std::string version_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
    std::string buffer_;
};

#endif