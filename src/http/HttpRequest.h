//
// Created by fzy on 23-7-5.
//

#ifndef TINYNETFLOW_HTTPREQUEST_H
#define TINYNETFLOW_HTTPREQUEST_H

#include "../base/Timestamp.h"

#include <map>
#include <cassert>
#include <cstdio>
#include <string>

namespace netflow::net {
/** copyable */
class HttpRequest {
public:
    /** HTTP 方法 */
    enum Method {
        kInvalid, kGet, kPost, kHead, kPut, kDelete
    };
    enum Version {
        kUnknown, kHttp10, kHttp11
    };
    HttpRequest()
        : method_(kInvalid),
          version_(kUnknown)
    {}

    void setVersion(Version n) { version_ = n; }
    Version getVersion() const { return version_; }

    bool setMethod(const char* start, const char* end) {
        assert(method_ == kInvalid);
        std::string m(start, end);
        if ( m == "GET")
        {
            method_ = kGet;
        }
        else if (m == "POST")
        {
            method_ = kPost;
        }
        else if (m == "HEAD")
        {
            method_ = kHead;
        }
        else if (m == "PUT")
        {
            method_ = kPut;
        }
        else if (m == "DELETE")
        {
            method_ = kDelete;
        }
        else
        {
            method_ = kInvalid;
        }
        return method_ != kInvalid;
    }
    Method getMethod() const { return method_; }

    const char* methodString() const {
        const char* result = "UNKNOWN";
        switch(method_)
        {
            case kGet:
                result = "GET";
                break;
            case kPost:
                result = "POST";
                break;
            case kHead:
                result = "HEAD";
                break;
            case kPut:
                result = "PUT";
                break;
            case kDelete:
                result = "DELETE";
                break;
            default:
                break;
        }
        return result;
    }

    void setPath(const char* start, const char* end) {
        path_.assign(start,end);
    }
    std::string getPath() const { return path_; }

    void setQuery(const char* start, const char* end) {
        query_.assign(start,end);
    }
    std::string getQuery() const { return query_; }

    void setReceiveTime(base::Timestamp t) { receiveTime_ = t; }
    base::Timestamp getReceiveTime() const { return receiveTime_; }
    /** colon:冒号 */
    void addHeader(const char* start, const char* colon, const char* end) {
        std::string field(start,colon);
        ++colon;
        /** 跳过中间空的部分 */
        while (colon < end && isspace(*colon)) {
            ++colon;
        }
        std::string value(colon, end);
        /** 将尾部不断收缩，去除尾部空余的部分 */
        while (!value.empty() && isspace(value[value.size() - 1])) {
            value.resize(value.size() - 1);
        }
        headers_[field] = value;
    }

    std::string getHeader(const std::string& field) const {
        std::string result;
        std::map<std::string, std::string>::const_iterator it = headers_.find(field);
        if (it != headers_.end()) {
            result = it->second;  /** 取出value */
        }
        return result;
    }

    const std::map<std::string, std::string>& getHeaders() const { return headers_; }

    void swap(HttpRequest& that) {
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        receiveTime_.swap(that.receiveTime_);
        headers_.swap(that.headers_);
    }
private:
    Method method_;
    Version version_;
    std::string path_;
    std::string  query_; /** 查询 */
    base::Timestamp receiveTime_;
    std::map<std::string, std::string> headers_;  /** key: ?, value: header */

};
} // namespace netflow::net
#endif //TINYNETFLOW_HTTPREQUEST_H
