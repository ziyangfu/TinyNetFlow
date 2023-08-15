//
// Created by fzy on 23-7-6.
//

#include "src/mqtt/MqttClient.h"

#include "src/base/Logging.h"
#include "src/net/InetAddr.h"


using namespace netflow::net;
using namespace netflow::base;
using namespace std::placeholders;
/*!
 * \brief 消息流向： input buffer -> MqttHeaderCodec::onMessage(拆包) -> MqttClient::onMessage */
MqttClient::MqttClient(netflow::net::EventLoop *loop, const netflow::net::InetAddr &serverAddr,
                       const std::string &name)
       : client_(loop, serverAddr, name),  /** 建立 TCP 连接 */
         isConnected_(false),
         connection_(nullptr),
         mqttClientArgs_(std::make_unique<MqttClientArgs>()),
         mqttHeaderCodec_(std::bind(&MqttClient::onMessage, this, _1, _2, _3))
{
    setMqttDefaultArgs();
    client_.setConnectionCallback(std::bind(&MqttClient::onConnection, this, _1));
    client_.setMessageCallback(std::bind(&MqttHeaderCodec::onMessage, mqttHeaderCodec_, _1, _2, _3 ));
    client_.enableRetry();
}

void MqttClient::setMqttDefaultArgs() {
    mqttClientArgs_->protocolVersion = MQTT_PROTOCOL_V311;
    mqttClientArgs_->keepAlive = MQTT_DEFAULT_KEEPALIVE;
}
/*!
 * \brief 建立TCP长链接 */
int MqttClient::connect() {
    client_.connect();
}
/*!
 * \brief 重连 */
int MqttClient::reconnect() {
    client_.retry();
}
/*!
 * \brief 关闭客户端写，保留读 */
void MqttClient::disconnect() {
    /** 发送 MQTT disconnect 消息 */
    sendHeadOnly(MQTT_TYPE_DISCONNECT, 0);
    client_.disconnect();
}
/*!
 * \brief 断开TCP长链接 */
void MqttClient::stop() {
    client_.stop();
}

/*!
 * \brief TCP连接建立后的回调函数，发送登录验证信息 */
void MqttClient::onConnection(const netflow::net::TcpConnectionPtr &conn) {
    STREAM_INFO << conn->getPeerAddr().toIpPort() << "-> "
                << conn->getLocalAddr().toIpPort() << " is "
                << (conn->isConnected() ? "UP" : "DOWN");
    if (conn->isConnected()) {
        isConnected_ = true;
        connection_ = conn;
        mqttClientLogin();
    }
    else {
        connection_.reset();
    }
}

/*!
 * \brief MQTT 发布 */
int MqttClient::publish(const std::string &topic, const std::string &payload, int qos, int retain,
                        netflow::net::MqttClient::MqttCallback ackCallback) {
    MqttMessage msg;
    memset(&msg, 0, sizeof msg);
    msg.topic_len = topic.size();
    msg.topic = topic;
    msg.payload_len = payload.size();
    msg.payload = payload;
    msg.qos = qos;   /** 有默认值 */
    msg.retain = retain;
    return publish(msg, std::move(ackCallback));
}
/*!
 * \brief
 *  1. 构建 MQTT header
 *  2. 将header、topic以及mid组包，发出去
 *  3. 发送payload */
int MqttClient::publish(netflow::net::mqtt::MqttMessage& msg, netflow::net::MqttClient::MqttCallback ackCallback) {
    int16_t topic_len = static_cast<int16_t>(msg.topic_len);
    int payload_len = static_cast<int>(msg.payload_len);
    int len = 2 + topic_len + payload_len;
    if (msg.qos > 0)
        len += 2;  /** for mid */
    int16_t mid = 0;

    /** 造包 */
    MqttHead head;
    memset(&head, 0, sizeof head);
    head.type = MQTT_TYPE_PUBLISH;
    head.qos = msg.qos & 3;  /** & 0011: 只会出现 0， 1， 2 三种值 */
    head.retain = msg.retain;
    head.length = len;
    int bufferLen = mqttEstimateLength(head);
    bufferLen -= payload_len; /** 去除payload，先发送前面的 */
    int headLength = 0;
    int totalLength = 0;
    auto buffer_ = std::make_unique<Buffer>();
    {
        auto buf = std::make_unique<char[]>(bufferLen);
        headLength = mqttHeadPack(&head, buf.get());
        buffer_->append(buf.get(), headLength);
    }
    buffer_->appendInt16(topic_len);
    buffer_->append(msg.topic.c_str(), topic_len);
    if (msg.qos) {
        mid = mqttNextMid();
        buffer_->appendInt16(mid);
        totalLength += 2;
    }
    /** send: head + topic + mid */
    totalLength += headLength + 2 + topic_len;
    send(std::move(buffer_), totalLength);
    /** send:payload */
    send(msg.payload);
}

int MqttClient::subscribe(const char *topic, int qos) {
    int16_t topic_len = static_cast<int16_t>(strlen(topic));
    int len = 2 + 2 + topic_len + 1;

    MqttHead head;
    memset(&head, 0, sizeof head);
    head.type = MQTT_TYPE_SUBSCRIBE;
    head.qos = 1;
    head.length = len;
    int headLength = 0;
    auto buffer_ = std::make_unique<Buffer>();
    {
        int buflen = mqttEstimateLength(head);
        auto buf = std::make_unique<char[]>(buflen);
        headLength = mqttHeadPack(&head, buf.get());
        buffer_->append(buf.get(), headLength);
    }
    int16_t mid = mqttNextMid();
    buffer_->appendInt16(mid);
    buffer_->appendInt16(topic_len);
    buffer_->append(topic, topic_len);
    buffer_->appendInt8(static_cast<int8_t>(qos & 3));
    /** send : head + mid + topic + qos */
    int totalLength = headLength + len;
    send(std::move(buffer_), totalLength);
}

int MqttClient::unSubscribe(const char *topic, netflow::net::MqttClient::MqttCallback ackCallback) {
    int16_t topic_len = static_cast<int16_t>(strlen(topic));
    int len = 2 + 2 + topic_len;

    MqttHead head;
    memset(&head, 0, sizeof head);
    head.type = MQTT_TYPE_UNSUBSCRIBE;
    head.qos = 1;
    head.length = len;
    int bufLength = mqttEstimateLength(head);
    int headLength = 0;
    auto buffer_ = std::make_unique<Buffer>();
    {
        auto buf = std::make_unique<char[]>(bufLength);
        headLength = mqttHeadPack(&head, buf.get());
        buffer_->append(buf.get(), headLength);
    }
    int16_t mid = mqttNextMid();
    buffer_->appendInt16(mid);
    buffer_->appendInt16(topic_len);
    buffer_->append(topic, topic_len);
    /** send: head + mid + topic */
    int totalLength = headLength + len;
    send(std::move(buffer_), totalLength);
}

/*!
 * \brief 发送消息
 * \private */
void MqttClient::send(std::string &message) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (connection_) {
        mqttHeaderCodec_.send(connection_.get(), message);
    }
}
/*!
 * \brief 发送消息
 * \private */
void MqttClient::send(std::unique_ptr<Buffer> buffer, const int len) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (connection_) {
        mqttHeaderCodec_.send(connection_.get(), std::move(buffer), len);
    }
}

int MqttClient::sendHeadOnly(int type, int length) {
    MqttHead head;
    memset(&head, 0, sizeof head);
    head.type = type;
    head.length = length;
    char headBuf[8] = {0};
    int headLength = mqttHeadPack(&head, headBuf);
    auto buffer_ = std::make_unique<Buffer>();
    buffer_->append(headBuf, headLength);
    send(std::move(buffer_), headLength);
}

int MqttClient::sendHeadWithMid(int type, int16_t  mid) {
    MqttHead head;
    memset(&head, 0, sizeof head);
    head.type = type;
    if (head.type == MQTT_TYPE_PUBREL) {
        head.qos = 1;
    }
    head.length = 2;

    char headBuf[8] = {0};
    int headLength = mqttHeadPack(&head, headBuf);
    auto buffer_ = std::make_unique<Buffer>();
    buffer_->append(headBuf, headLength);
    buffer_->appendInt16(mid);
    send(std::move(buffer_), headLength + 2);

}

void MqttClient::sendPing() {
    sendHeadOnly(MQTT_TYPE_PINGREQ, 0);
}

int MqttClient::sendPong() {
    sendHeadOnly(MQTT_TYPE_PINGRESP, 0);
}


void MqttClient::close() {
    // pass
}
/*!
 * \brief 在完成TCP拆包后，完成MQTT协议的解析 */
void MqttClient::onMessage(const TcpConnectionPtr&, Buffer& buf, Timestamp receiveTime) {
    STREAM_TRACE << "Received a MQTT message！ receive time is: " << receiveTime.toString();
    mqttProtocolParse(buf);
    //mqttMessageCallback_(message);  /** 执行上层回调函数 */
}

/*!
 * MQTT_TYPE_CONNECT
 * 2 + protocol_name + 1 protocol_version + 1 conn_flags + 2 keepalive + 2 + [client_id] +
 * [2 + will_topic + 2 + will_payload] +
 * [2 + username] + [2 + password]
 */
int MqttClient::mqttClientLogin() {
    int len = 2 + 1 + 1 + 2 + 2;
    unsigned short  cid_len = 0,
                    will_topic_len = 0,
                    will_payload_len = 0,
                    username_len = 0,
                    password_len = 0;
    unsigned char conn_flags = 0;
    len += mqttClientArgs_->protocolVersion = MQTT_PROTOCOL_V31 ? 6 : 4;
    /** clientId 长度 */
    if (!mqttClientArgs_->clientId.empty()) {
        cid_len = static_cast<unsigned short>(mqttClientArgs_->clientId.size());
    }
    else {
        cid_len = 20;
        mqttClientArgs_->clientId = "randomString"; /** FIXME: 添加随机字符串 */
        STREAM_DEBUG << "MQTT clientId :  " << mqttClientArgs_->clientId;
    }
    len += cid_len;

    if (cid_len == 0) { mqttClientArgs_->cleanSession = 1; }
    if (mqttClientArgs_->cleanSession) {
        conn_flags |= MQTT_CONN_CLEAN_SESSION;
    }

    if (mqttClientArgs_->will && !mqttClientArgs_->will->topic.empty() && !mqttClientArgs_->will->payload.empty()) {
        will_topic_len = mqttClientArgs_->will->topic_len ?
                mqttClientArgs_->will->topic_len : static_cast<unsigned short>(mqttClientArgs_->will->topic.size());
        will_payload_len = mqttClientArgs_->will->payload_len ?
                mqttClientArgs_->will->payload_len : static_cast<unsigned short>(mqttClientArgs_->will->payload.size());
        if (will_topic_len && will_payload_len) {
            conn_flags |= MQTT_CONN_HAS_WILL;
            conn_flags |= ((mqttClientArgs_->will->qos & 3) << 3);
            if (mqttClientArgs_->will->retain) {
                conn_flags |= MQTT_CONN_WILL_RETAIN;
            }
            len += 2 + will_topic_len;
            len += 2 + will_payload_len;
        }
    }
    if (!mqttClientArgs_->userName.empty()) {
        username_len = static_cast<unsigned short>(mqttClientArgs_->userName.size());
        if (username_len) {
            conn_flags |= MQTT_CONN_HAS_USERNAME;
            len += 2 + username_len;
        }
    }
    if (!mqttClientArgs_->password.empty()) {
        password_len = static_cast<unsigned short>(mqttClientArgs_->password.size());
        if (password_len) {
            conn_flags |= MQTT_CONN_HAS_PASSWORD;
            len += 2 + password_len;
        }
    }
    /** 造包 */
    MqttHead head;
    memset(&head, 0, sizeof(head));
    head.type = MQTT_TYPE_CONNECT;
    head.length = len;
    int bufLength = mqttEstimateLength(head);
    auto buffer_ = std::make_unique<Buffer>();
    //auto buf = std::unique_ptr<unsigned char>(new unsigned char[bufLength]);
    {
        std::unique_ptr<char[]> buf = std::make_unique<char[]>(bufLength);
        char* p = buf.get();
        int headLength = mqttHeadPack(&head, p); /** 写入MQTT头 */
        buffer_->append(p,headLength);
    }
    // TODO: Not implement MQTT_PROTOCOL_V5
    if (mqttClientArgs_->protocolVersion == MQTT_PROTOCOL_V31) {
        buffer_->appendInt8(6);
        buffer_->append(MQTT_PROTOCOL_NAME_v31, 6);
    } else {
        buffer_->appendInt8(4);
        buffer_->append(MQTT_PROTOCOL_NAME, 4);
    }
    buffer_->appendInt8(static_cast<int8_t>(mqttClientArgs_->protocolVersion));
    buffer_->appendInt8(static_cast<int8_t>(conn_flags));
    buffer_->appendInt16(static_cast<int16_t>(mqttClientArgs_->keepAlive));
    buffer_->appendInt16(static_cast<int16_t>(cid_len));

    if (cid_len > 0) {
        buffer_->append(mqttClientArgs_->clientId.c_str(), cid_len);
    }
    if (conn_flags & MQTT_CONN_HAS_WILL) {
        buffer_->appendInt16(static_cast<int16_t>(will_payload_len));
        buffer_->append(mqttClientArgs_->will->topic.c_str(), will_topic_len);
        buffer_->appendInt16(static_cast<int16_t>(will_topic_len));
        buffer_->append(mqttClientArgs_->will->payload.c_str(), will_payload_len);
    }
    if (conn_flags & MQTT_CONN_HAS_USERNAME) {
        buffer_->appendInt16(static_cast<int16_t>(username_len));
        buffer_->append(mqttClientArgs_->userName.c_str(), username_len);
    }
    if (conn_flags & MQTT_CONN_HAS_PASSWORD) {
        buffer_->appendInt16(static_cast<int16_t>(password_len));
        buffer_->append(mqttClientArgs_->password.c_str(), password_len);
    }
    /**  发送登录信息 */
    send(std::move(buffer_), bufLength);
}

/*!
 * \brief mid累加
 * \static
 * \private */
int16_t MqttClient::mqttNextMid() {
    static int16_t mid = 0;
    return ++mid;
}
/*!
 * \brief MQTT 消息的解析 */
std::string &MqttClient::mqttProtocolParse(Buffer& buf) {
    auto mqttClient_ = std::make_unique<MqttClientArgs>();
    memset(&mqttClient_->head, 0, sizeof(mqttClient_->head));
    int headLen = 0;
    {
        const char* p = buf.peek();
        headLen = mqttHeadUnpack(&mqttClient_->head, p, static_cast<int>(buf.readableBytes()));
        assert(headLen > 0);
    }

    switch (mqttClient_->head.type) {
        case MQTT_TYPE_CONNACK:
        {
            if (mqttClient_->head.length < 2) {
                STREAM_ERROR << "error in MQTT_TYPE_CONNACK ";
                connection_->shutdown();
            }
            int8_t conn_flags = buf.readInt8();  /** 连接确认标志 connectAckFlags */
            int8_t rc = buf.readInt8();
            if (rc != MQTT_CONNACK_ACCEPTED) {
                mqttClient_->error = rc;
                STREAM_ERROR << "MQTT CONNACK error = " << mqttClient_->error;
                connection_->shutdown();
            }
            mqttClient_->connected = 1;  /** MQTT连接成功 */
            if (mqttClient_->keepAlive) {
                setHeartbeat(mqttClient_->keepAlive * 1000);
            }
        }
            break;
        case MQTT_TYPE_PUBLISH:
        {
            if (mqttClient_->head.length < 2) {
                STREAM_ERROR << "error in MQTT_TYPE_PUBLISH ";
                connection_->shutdown();
            }
            memset(&mqttClient_->message, 0, sizeof(mqttClient_->message));
            mqttClient_->message.topic_len = buf.readInt16();
            mqttClient_->message.topic = std::string(buf.peek(), mqttClient_->message.topic_len);
            buf.retrieve(mqttClient_->message.topic_len);
            if (mqttClient_->head.qos > 0) {
                mqttClient_->mid = buf.readInt16();
            }
            mqttClient_->message.payload_len = buf.readableBytes();
            mqttClient_->message.payload = buf.retrieveAllAsString();
            mqttClient_->message.qos = mqttClient_->head.qos;
            if (mqttClient_->message.qos == 0) {
                /** 不做任何事 */
            }
            else if (mqttClient_->message.qos == 1) {
                sendHeadWithMid(MQTT_TYPE_PUBACK, mqttClient_->mid);
            }
            else if (mqttClient_->message.qos == 2) {
                sendHeadWithMid(MQTT_TYPE_PUBREC, mqttClient_->mid);
            }
        }
            break;
        case MQTT_TYPE_PUBACK:
        case MQTT_TYPE_PUBREC:
        case MQTT_TYPE_PUBREL:
        case MQTT_TYPE_PUBCOMP:
        {
            if (mqttClient_->head.length < 2) {
                STREAM_ERROR << "error in MQTT_TYPE_PUBACK ";
                connection_->shutdown();
            }
            mqttClient_->mid = buf.readInt16();
            if (mqttClient_->head.type == MQTT_TYPE_PUBREC) {
                sendHeadWithMid(MQTT_TYPE_PUBREL, mqttClient_->mid);
            }
            if (mqttClient_->head.type == MQTT_TYPE_PUBREL) {
                sendHeadWithMid(MQTT_TYPE_PUBCOMP, mqttClient_->mid);
            }
        }
            break;
        case MQTT_TYPE_SUBACK:
        {
            if (mqttClient_->head.length < 2) {
                STREAM_ERROR << "error in MQTT_TYPE_SUBACK ";
                connection_->shutdown();
            }
            mqttClient_->mid = buf.readInt16();
        }
            break;
        case MQTT_TYPE_UNSUBACK:
        {
            if (mqttClient_->head.length < 2) {
                STREAM_ERROR << "error in MQTT_TYPE_UNSUBACK ";
                connection_->shutdown();
            }
            mqttClient_->mid = buf.readInt16();
        }
            break;
        case MQTT_TYPE_PINGREQ:
            sendPong();
            break;
        case MQTT_TYPE_PINGRESP:
            mqttClient_->pingCnt = 0;
            break;
        case MQTT_TYPE_DISCONNECT:
            connection_->shutdown();
            break;
        default:
        {
            STREAM_ERROR << "MQTT client received wrong type, the type is : " << mqttClient_->head.type;
        }
            break;
    }
}




/** 应用层心跳 */
void MqttClient::setHeartbeat(int intervalMs) {
    assert(intervalMs > 0);
    /** 增加定时器，定时器到期时，发送 ping */
    sendPing();
}

