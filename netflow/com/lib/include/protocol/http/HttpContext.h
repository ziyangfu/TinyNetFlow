#ifndef COM_PROTOCOL_HTTP_HTTP_CONTEXT_H
#define COM_PROTOCOL_HTTP_HTTP_CONTEXT_H
#include "protocol/http/HttpRequest.h"

namespace osadaptor::net {
    class Buffer;  /** 前置声明 */
}

namespace com {
/** copyable */
// 对HTTP请求报文的解析
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
    bool parseRequest(osadaptor::net::Buffer* buf, osadaptor::time::Timestamp receiveTime);

    bool isGotAll() const { return state_ == kGotAll; }

    void reset() {
        state_ = kExpectRequestLine;
        HttpRequest dummy;
        request_.swap(dummy);  /** 换成空的request */
    }
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
} // namespace com



#endif //COM_PROTOCOL_HTTP_HTTP_CONTEXT_H
