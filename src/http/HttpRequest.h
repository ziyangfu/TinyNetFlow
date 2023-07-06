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

    bool setMethod(const char* start, const char* end) {}
    Method getMethod() const { return method_; }

    const char* methodString() const {}

    void setPath(const char* start, const char* end) {}
    std::string getPath() const { return path_; }

    void setQuery(const char* start, const char* end) {}
    std::string getQuery() const { return query_; }

    void setReceiveTime(base::Timestamp t) { receiveTime_ = t; }
    base::Timestamp getReceiveTime() const { return receiveTime_; }
    /** colon:冒号 */
    void addHeader(const char* start, const char* colon, const char* end) {

    }
    std::string getHeader(const std::string& field) const {}

    const std::map<std::string, std::string>& getHeaders() const { return headers_; }

    void swap(HttpRequest& that) {}
private:
    Method method_;
    Version version_;
    std::string path_;
    std::string  query_; /** 查询 */
    base::Timestamp receiveTime_;
    std::map<std::string, std::string> headers_;

};
} // namespace netflow::net
#endif //TINYNETFLOW_HTTPREQUEST_H
