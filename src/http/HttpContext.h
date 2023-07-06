//
// Created by fzy on 23-7-5.
//

#ifndef TINYNETFLOW_HTTPCONTEXT_H
#define TINYNETFLOW_HTTPCONTEXT_H

#include "HttpRequest.h"

namespace netflow::net {

class Buffer;  /** 前置声明 */
/** copyable */
class HttpContext {
public:
    enum HttpRequestParseState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll
    };
    HttpContext()
        : state_(kExpectRequestLine)
    {}
    bool parseRequest(Buffer* buf, base::Timestamp receiveTime);

    bool isGotAll() const { return state_ == kGotAll; }

    void reset() {}
    /** 不可修改 */
    const HttpRequest& getRequest() const { return request_; }
    /** 重载，可修改 */
    HttpRequest& getRequest() { return request_; }
private:
    bool processRequestLine(const char* begin, const char* end);
private:
    HttpRequestParseState state_;
    HttpRequest request_;
};
} // namespace netflow::net



#endif //TINYNETFLOW_HTTPCONTEXT_H
