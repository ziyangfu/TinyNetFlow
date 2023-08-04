//
// Created by fzy on 23-8-1.
//
#include "MqttHeaderCodec.h"
#include "MqttProtocol.h"

using namespace netflow::net::mqtt;
/*!
 * \brief TCP拆包 */
void MqttHeaderCodec::onMessage(const netflow::net::TcpConnectionPtr &conn, netflow::net::Buffer *buf,
                                netflow::base::Timestamp receiveTime) {
     /** 按照协议长度方式拆包 */
    int kHeadLength = 2;               /** MQTT 最小固定head长度 */
    int kBodyLength = 0;
    int kPackageLength = kHeadLength;   /** 一条完整的MQTT消息长度 */
    const char* pVariate = nullptr;  /** 剩余长度首位置 */
    while(buf->readableBytes() >= kHeadLength) {
        kBodyLength = 0;
        const char* pStartRead = buf->peek();  /** 数据可读首位置 */
        pVariate = pStartRead + 1;   /** 剩余长度在首位置+1处，前一个字节为head部分 */
        /** 在这里仅赋初值，表示从剩余长度开始，已经有了多少字节
            剩余长度占用几个字节（至少1个，最多4个）*/
        int variateBytes = static_cast<int>(buf->readableBytes());
        /**  计算出body长度，已经剩余长度variateBytes
             当readBytes不够长度时，将variateBytes == 0；后面判断break
             kBodyLength 即表示剩下有多少个字节，根据该数据拆包，常见做法 */
        kBodyLength = variateDecode(pVariate, &variateBytes);  /** FIXME: 是否有字节序问题？ */
        if (variateBytes == 0) {
            break;
        }
        if (variateBytes == -1) {
            STREAM_ERROR << "variate is too big!";
            conn->shutdown();
        }
        /** head固定长度 1字节，+可变长度，最大4个字节，kHeadLength已经多算了1个字节的剩余长度，所以需要减去1 */
        kHeadLength = kHeadLength - 1 + variateBytes;
        kPackageLength = kHeadLength + kBodyLength;

        /** 接收到了一个完整的MQTT包 */
        if (static_cast<int>(buf->readableBytes()) >= kPackageLength) {
            Buffer message_buf;
            message_buf.append(buf->peek(), kPackageLength);  /** 目前是写buf */
            messageCallback_(conn, message_buf, receiveTime);  /** 组成完整消息后，执行消息回调 */
            buf->retrieve(kPackageLength);
        }
    }
}

void MqttHeaderCodec::send(netflow::net::TcpConnection *conn, const std::string &message) {
    Buffer buffer;
    const char* data = &(*message.begin());
    buffer.append(data, message.size());  /** 添加消息 */
    //int32_t len = static_cast<int32_t>(message.size());
    // int32_t be32 = htonl(len);
    //buffer.prepend(&be32, sizeof be32);  /** 在消息前面添加消息长度,简单的拆包规则 */
    conn->send(&buffer);
}

void MqttHeaderCodec::send(netflow::net::TcpConnection *conn, std::unique_ptr<Buffer> buffer, const int len) {
    //int32_t be32 = htonl(len);
    //buffer->prepend(&be32, sizeof be32);
    conn->send(buffer.get());
}