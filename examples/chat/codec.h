//
// Created by fzy on 23-7-5.
//

#ifndef TINYNETFLOW_CODEC_H
#define TINYNETFLOW_CODEC_H

#include "../../src/base/Logging.h"
#include "../../netflow/net//Buffer.h"
#include "../../netflow/net//TcpConnection.h"

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
        : messageCallback_(cb)
    {}

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime) {
        /** 拆包的过程，这实际是数据回调，应该做封装，做到用户无感，用户只需要关心消息的回调 */
        /** |  length(4 byte)  |  message(N byte)    |
         *  |                  |                readerIndex */

        /** 必须用while，保证数据的完整性，收到一个字节的数据，就触发了消息回调，此时拿到的消息不完整 */
        while (buf->readableBytes() >= kHeaderLen) {    /** kHeaderLen == 4 */
            const void* data = buf->peek();   /** readerIndex 指针，开始读数据的位置 */
            int32_t be32 = *static_cast<const int32_t*>(data);
            const int32_t len = ntohl(be32);  /** 读到的是消息的长度，即发送时写入的int32_t长度 */
            if (len > 65536 || len < 0){
                STREAM_ERROR << "invalid length " << len;
                conn->shutdown();
                break;
            }
            else if (buf->readableBytes() >= len + kHeaderLen) {   /** 保证消息的完整性 */
                buf->retrieve(kHeaderLen);  /** 向右移动4字节，指向消息的起始点 */
                std::string message(buf->peek(), len);
                messageCallback_(conn, message, receiveTime);  /** 组成完整消息后，执行消息回调 */
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
        buffer.append(data, message.size());  /** 添加消息 */
        int32_t len = static_cast<int32_t>(message.size());
        int32_t be32 = htonl(len);
        buffer.prepend(&be32, sizeof be32);  /** 在消息前面添加消息长度 */
        conn->send(&buffer);
    }

private:
    StringMessageCallabck messageCallback_;
    const static size_t kHeaderLen = sizeof(int32_t);
};

#endif //TINYNETFLOW_CODEC_H
