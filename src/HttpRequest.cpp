#include "HttpRequest.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstring>

HttpRequest::HttpRequest() {
    reset();
}

void HttpRequest::reset() {
    state_ = PARSE_REQUEST_LINE;
    method_.clear();
    path_.clear();
    version_.clear();
    headers_.clear();
    body_.clear();
    buffer_.clear();
}

bool HttpRequest::parse(const char* data, int len) {
    buffer_.append(data, len);
    int pos = 0;

    while (true) {
        if (state_ == PARSE_REQUEST_LINE) {
            size_t end = buffer_.find("\n", pos);
            if (end == std::string::npos) break;

            std::string line = buffer_.substr(pos, end - pos);
            if (!parseRequestLine(line)) {
                state_ = PARSE_ERROR;
                return false;
            }

            pos = end + 1;
            state_ = PARSE_HEADER;
        }
        else if (state_ == PARSE_HEADER) {
            size_t end = buffer_.find("\n", pos);
            if (end == std::string::npos) break;

            if (end == pos) {
                pos = end + 1;
                state_ = PARSE_BODY;
                break;
            }

            std::string line = buffer_.substr(pos, end - pos);
            if (!parseHeaderLine(line)) {
                state_ = PARSE_ERROR;
                return false;
            }
            pos = end + 1;
        }
        else if (state_ == PARSE_BODY) {
            int cl = getContentLength();
            if (cl <= 0) {
                state_ = PARSE_DONE;
                break;
            }

            if ((int)buffer_.size() - pos >= cl) {
                body_ = buffer_.substr(pos, cl);
                state_ = PARSE_DONE;
                break;
            }
            else break;
        }
        else break;
    }

    if (pos > 0) {
        buffer_ = buffer_.substr(pos);
    }

    return state_ == PARSE_DONE;
}

bool HttpRequest::parseRequestLine(const std::string& line) {
    std::istringstream iss(line);
    std::string m, p, v;
    iss >> m >> p >> v;

    if (m.empty() || p.empty() || v.empty()) return false;

    method_ = m;
    path_ = p;
    version_ = v;
    return true;
}

bool HttpRequest::parseHeaderLine(const std::string& line) {
    size_t colon = line.find(':');
    if (colon == std::string::npos) return false;

    std::string key = line.substr(0, colon);
    std::string val = line.substr(colon + 1);

    key.erase(0, key.find_first_not_of(" \t"));
    key.erase(key.find_last_not_of(" \t") + 1);
    val.erase(0, val.find_first_not_of(" \t"));
    val.erase(val.find_last_not_of(" \t") + 1);

    std::transform(key.begin(), key.end(), key.begin(), tolower);
    headers_[key] = val;
    return true;
}

int HttpRequest::getContentLength() const {
    std::unordered_map<std::string, std::string>::const_iterator it;
    it = headers_.find("content-length");
    if (it == headers_.end()) return 0;
    return atoi(it->second.c_str());
}