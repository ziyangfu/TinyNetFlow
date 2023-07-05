//
// Created by fzy on 23-7-5.
//

#ifndef TINYNETFLOW_CODEC_H
#define TINYNETFLOW_CODEC_H

#include "../../src/base/Logging.h"
#include "../../src/net/Buffer.h"
#include "../../src/net/TcpConnection.h"

#include <string>
#include <arpa/inet.h>

using namespace netflow::base;
using namespace netflow::net;
class LengthHeaderCodec {
public:
    using StringMessageCallabck = std::function<void (const TcpConnectionPtr&,
                                                      const std::string& message,
                                                      Timestamp)>;
    explicit LengthHeaderCodec(const StringMessageCallabck& cb)
        : messageCallabck_(cb)
    {}

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
        while (buf->readableBytes() >= kHeaderLen) {
            const void* data = buf->peek();
            int32_t be32 = *static_cast<const int32_t*>(data);
            const int32_t len = ntohl(be32);  /** 为什么是len*/
            if (len > 65536 || len < 0){
                STREAM_ERROR << "invalid length " << len;
                conn->shutdown();
                break;
            }
            else if (buf->readableBytes() >= len + kHeaderLen) {
                buf->retrieve(kHeaderLen);
                std::string message(buf->peek(), len);
                messageCallabck_(conn, message, receiveTime);
                buf->retrieve(len);
            }
            else {
                break;
            }
        }
    }

    void send(TcpConnection* conn, const std::string& message){
        Buffer buffer;
        const char* data = &(*message.begin());
        buffer.append(data, message.size());
        int32_t len = static_cast<int32_t>(message.size());
        int32_t be32 = htonl(len);
        buffer.prepend(&be32, sizeof be32);
        conn->send(&buffer);
    }

private:
    StringMessageCallabck messageCallabck_;
    const static size_t kHeaderLen = sizeof(int32_t);
};

#endif //TINYNETFLOW_CODEC_H
