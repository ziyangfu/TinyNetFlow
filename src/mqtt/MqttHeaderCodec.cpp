//
// Created by fzy on 23-8-1.
//
#include "MqttHeaderCodec.h"

using namespace netflow::net::mqtt;
/*!
 * \brief TCP拆包 */
void MqttHeaderCodec::onMessage(const netflow::net::TcpConnectionPtr &conn, netflow::net::Buffer *buf,
                                netflow::base::Timestamp receiveTime) {
    /*
    mqtt_client_t* cli = (mqtt_client_t*)hevent_userdata(io);
    unsigned char* p = (unsigned char*)buf;
    unsigned char* end = p + len;
    memset(&cli->head, 0, sizeof(mqtt_head_t));
    int headlen = mqtt_head_unpack(&cli->head, p, len);
    if (headlen <= 0) return;
    p += headlen;
    switch (cli->head.type) {
    // case MQTT_TYPE_CONNECT:
    case MQTT_TYPE_CONNACK:
    {
        if (cli->head.length < 2) {
            hloge("MQTT CONNACK malformed!");
            hio_close(io);
            return;
        }
        unsigned char conn_flags = 0, rc = 0;
        POP8(p, conn_flags);
        POP8(p, rc);
        if (rc != MQTT_CONNACK_ACCEPTED) {
            cli->error = rc;
            hloge("MQTT CONNACK error=%d", cli->error);
            hio_close(io);
            return;
        }
        cli->connected = 1;
        if (cli->keepalive) {
            hio_set_heartbeat(io, cli->keepalive * 1000, mqtt_send_ping);
        }
    }
        break;
    case MQTT_TYPE_PUBLISH:
    {
        if (cli->head.length < 2) {
            hloge("MQTT PUBLISH malformed!");
            hio_close(io);
            return;
        }
        memset(&cli->message, 0, sizeof(mqtt_message_t));
        POP16(p, cli->message.topic_len);
        if (end - p < cli->message.topic_len) {
            hloge("MQTT PUBLISH malformed!");
            hio_close(io);
            return;
        }
        // NOTE: Not deep copy
        cli->message.topic = (char*)p;
        p += cli->message.topic_len;
        if (cli->head.qos > 0) {
            if (end - p < 2) {
                hloge("MQTT PUBLISH malformed!");
                hio_close(io);
                return;
            }
            POP16(p, cli->mid);
        }
        cli->message.payload_len = end - p;
        if (cli->message.payload_len > 0) {
            // NOTE: Not deep copy
            cli->message.payload = (char*)p;
        }
        cli->message.qos = cli->head.qos;
        if (cli->message.qos == 0) {
            // Do nothing
        } else if (cli->message.qos == 1) {
            mqtt_send_head_with_mid(io, MQTT_TYPE_PUBACK, cli->mid);
        } else if (cli->message.qos == 2) {
            mqtt_send_head_with_mid(io, MQTT_TYPE_PUBREC, cli->mid);
        }
    }
        break;
    case MQTT_TYPE_PUBACK:
    case MQTT_TYPE_PUBREC:
    case MQTT_TYPE_PUBREL:
    case MQTT_TYPE_PUBCOMP:
    {
        if (cli->head.length < 2) {
            hloge("MQTT PUBACK malformed!");
            hio_close(io);
            return;
        }
        POP16(p, cli->mid);
        if (cli->head.type == MQTT_TYPE_PUBREC) {
            mqtt_send_head_with_mid(io, MQTT_TYPE_PUBREL, cli->mid);
        } else if (cli->head.type == MQTT_TYPE_PUBREL) {
            mqtt_send_head_with_mid(io, MQTT_TYPE_PUBCOMP, cli->mid);
        }
    }
        break;
    // case MQTT_TYPE_SUBSCRIBE:
    //     break;
    case MQTT_TYPE_SUBACK:
    {
        if (cli->head.length < 2) {
            hloge("MQTT SUBACK malformed!");
            hio_close(io);
            return;
        }
        POP16(p, cli->mid);
    }
        break;
    // case MQTT_TYPE_UNSUBSCRIBE:
    //     break;
    case MQTT_TYPE_UNSUBACK:
    {
        if (cli->head.length < 2) {
            hloge("MQTT UNSUBACK malformed!");
            hio_close(io);
            return;
        }
        POP16(p, cli->mid);
    }
        break;
    case MQTT_TYPE_PINGREQ:
        // printf("recv ping\n");
        // printf("send pong\n");
        mqtt_send_pong(io);
        return;
    case MQTT_TYPE_PINGRESP:
        // printf("recv pong\n");
        cli->ping_cnt = 0;
        return;
    case MQTT_TYPE_DISCONNECT:
        hio_close(io);
        return;
    default:
        hloge("MQTT client received wrong type=%d", (int)cli->head.type);
        hio_close(io);
        return;
    }

    if (cli->cb) {
        cli->cb(cli, cli->head.type);
    }
}
     * */

    /**
     *  头部长度                bodyOffset
     *  头部长度字段偏移量       lengthFieldOffset
     *  头部长度字段所占字节数    lengthFieldBytes
     *  头部长度字段编码方式，支持可变，大小端三种编码方式，通常使用大端字节序
     *  */


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