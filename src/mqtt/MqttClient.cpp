//
// Created by fzy on 23-7-6.
//
#include "src/mqtt/MqttClient.h"

#include "src/base/Logging.h"
#include "src/net/InetAddr.h"

#include <random>
#include <iostream>

using namespace netflow::net;
using namespace netflow::base;
using namespace std::placeholders;

/*!
 * \brief 消息流向： input buffer -> MqttHeaderCodec::onMessage(拆包) -> MqttClient::onMessage */
MqttClient::MqttClient(netflow::net::EventLoop *loop, const netflow::net::InetAddr &serverAddr,
                       const std::string &name)
       : client_(loop, serverAddr, name),  /** 建立 TCP 连接 */
         loop_(loop),
         isConnected_(false),
         connection_(nullptr),
         mqttContext_(std::make_unique<MqttContext>()),
         mqttHeaderCodec_(std::bind(&MqttClient::onMessage, this, _1, _2, _3))
{
    client_.setConnectionCallback(std::bind(&MqttClient::onConnection, this, _1));
    client_.setMessageCallback(std::bind(&MqttHeaderCodec::onMessage, mqttHeaderCodec_, _1, _2, _3 ));
    // client_.enableRetry();
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
    STREAM_INFO << "TCP connect: "
                << conn->getPeerAddr().toIpPort() << "-> "
                << conn->getLocalAddr().toIpPort() << " is "
                << (conn->isConnected() ? "UP" : "DOWN");
    if (conn->isConnected()) {
        //isConnected_ = true;
        connection_ = conn;
        mqttClientLogin();
    }
    else {
        connection_.reset();
    }
}

/*!
 * \brief MQTT 发布 */
int MqttClient::publish(const std::string &topic, const std::string &payload, int8_t qos, int8_t retain) {
    mqttContext_->mqttMessageInit();
    auto msgPtr = mqttContext_->getMessagePtr();
    msgPtr->topicLength = static_cast<int16_t>(topic.size());
    msgPtr->topic = topic;
    msgPtr->payloadLength = static_cast<int16_t>(payload.size());
    msgPtr->payload = payload;
    msgPtr->qos = qos;
    msgPtr->retain = retain;
    return publish(msgPtr);

}

int MqttClient::publish(const char *topic, const char *payload, int8_t qos, int8_t retain) {
    mqttContext_->mqttMessageInit();
    auto msgPtr = mqttContext_->getMessagePtr();

    msgPtr->topicLength = strlen(topic);
    msgPtr->topic = topic;
    msgPtr->payloadLength = strlen(payload);
    msgPtr->payload = payload;
    msgPtr->qos = qos;   /** 有默认值 */
    msgPtr->retain = retain;
    return publish(msgPtr);
}
/*!
 * \brief
 *  1. 构建 MQTT header
 *  2. 将header、topic以及mid组包，发出去
 *  3. 发送payload */
int MqttClient::publish(std::shared_ptr<MqttContext::MqttMessage> msgPtr) {
    int len = 2 + msgPtr->topicLength + msgPtr->payloadLength;
    if (msgPtr->qos > 0)
        len += 2;  /** for mid */
    int16_t mid = 0;

    /** 造包 */
    mqttContext_->mqttHeadInit();
    auto& head = mqttContext_->getHeader();   /** 返回对象的引用，若没有&，则实际是赋值副本 */

    head.type = MQTT_TYPE_PUBLISH;
    head.qos = msgPtr->qos & 3;  /** & 0011: 只会出现 0， 1， 2 三种值 */

    head.retain = msgPtr->retain;
    head.length = len;
    int bufferLen = mqttContext_->mqttEstimateLength();
    bufferLen -= msgPtr->payloadLength; /** 去除payload，先发送前面的 */
    int headLength = 0;
    int totalLength = 0;
    auto buffer_ = std::make_unique<Buffer>();
    {
        auto buf = std::make_unique<char[]>(bufferLen);
        headLength = mqttContext_->mqttHeadPack(buf.get());
        buffer_->append(buf.get(), headLength);
    }
    buffer_->appendInt16(msgPtr->topicLength);
    buffer_->append(msgPtr->topic);
    if (msgPtr->qos) {
        mid = mqttNextMid();
        buffer_->appendInt16(mid);
        totalLength += 2;
    }
    /** send: head + topic + mid */
    //totalLength += headLength + 2 + topic_len;
    send(std::move(buffer_));
    /** send:payload */
    send(msgPtr->payload);
}

int MqttClient::subscribe(const char *topic, int qos) {
    int16_t topic_len = static_cast<int16_t>(strlen(topic));
    mqttContext_->mqttHeadInit();
    auto& head = mqttContext_->getHeader();
    head.type = MQTT_TYPE_SUBSCRIBE;
    head.qos = 1;
    head.length = 2 + 2 + topic_len + 1;
    int headLength = 0;
    auto buffer_ = std::make_unique<Buffer>();
    {
        int buflen = mqttContext_->mqttEstimateLength();
        auto buf = std::make_unique<char[]>(buflen);
        headLength = mqttContext_->mqttHeadPack(buf.get());
        buffer_->append(buf.get(), headLength);
    }
    int16_t mid = mqttNextMid();
    buffer_->appendInt16(mid);
    buffer_->appendInt16(topic_len);
    buffer_->append(topic, topic_len);
    buffer_->appendInt8(static_cast<int8_t>(qos & 3));
    /** send : head + mid + topic + qos */
    send(std::move(buffer_));
}

int MqttClient::subscribe(const std::string &topic) {
    subscribe(topic.c_str());
}

int MqttClient::unSubscribe(const char *topic) {
    int16_t topic_len = static_cast<int16_t>(strlen(topic));
    int len = 2 + 2 + topic_len;
    mqttContext_->mqttHeadInit();
    auto& head = mqttContext_->getHeader();
    head.type = MQTT_TYPE_UNSUBSCRIBE;
    head.qos = 1;
    head.length = len;
    int bufLength = mqttContext_->mqttEstimateLength();
    int headLength = 0;
    auto buffer_ = std::make_unique<Buffer>();
    {
        auto buf = std::make_unique<char[]>(bufLength);
        headLength = mqttContext_->mqttHeadPack(buf.get());
        buffer_->append(buf.get(), headLength);
    }
    int16_t mid = mqttNextMid();
    buffer_->appendInt16(mid);
    buffer_->appendInt16(topic_len);
    buffer_->append(topic, topic_len);
    /** send: head + mid + topic */
    int totalLength = headLength + len;
    send(std::move(buffer_));
}

int MqttClient::unSubscribe(const std::string &topic) {
    unSubscribe(topic.c_str());
}
/*!
 * \brief 发送消息
 * \private */
void MqttClient::send(std::string &message) {
    send(message.c_str(), message.size());
}

void MqttClient::send(const char *message, int length) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(connection_) {
        mqttHeaderCodec_.send(connection_.get(), message, length);
    }

}
/*!
 * \brief 发送消息
 * \private */
void MqttClient::send(std::unique_ptr<Buffer> buffer) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (connection_) {
        mqttHeaderCodec_.send(connection_.get(), std::move(buffer));
    }
}

int MqttClient::sendHeadOnly(int8_t type, int length) {
    mqttContext_->mqttHeadInit();
    auto& head = mqttContext_->getHeader();

    head.type = type;
    head.length = length;
    char headBuf[8] = {0};
    int headLength = mqttContext_->mqttHeadPack(headBuf);
    auto buffer_ = std::make_unique<Buffer>();
    buffer_->append(headBuf, headLength);
    send(std::move(buffer_));
}

int MqttClient::sendHeadWithMid(int8_t type, int16_t  mid) {
    mqttContext_->mqttHeadInit();
    auto& head = mqttContext_->getHeader();

    head.type = type;
    if (head.type == MQTT_TYPE_PUBREL) {
        head.qos = 1;
    }
    head.length = 2;

    char headBuf[8] = {0};
    int headLength = mqttContext_->mqttHeadPack(headBuf);
    auto buffer_ = std::make_unique<Buffer>();
    buffer_->append(headBuf, headLength);
    buffer_->appendInt16(mid);
    send(std::move(buffer_));

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
    STREAM_INFO << "Received a MQTT message！ receive time is: " << receiveTime.toString();
    mqttProtocolParse(buf);
}

/*!
 * MQTT_TYPE_CONNECT
 * 2 + protocol_name + 1 protocol_version + 1 conn_flags + 2 keepalive + 2 + [client_id] +
 * [2 + will_topic + 2 + will_payload] +
 * [2 + username] + [2 + password]
 */
int MqttClient::mqttClientLogin() {
    int len = 2 + 1 + 1 + 2 + 2;
    int16_t clientId_len = 0,
            username_len = 0,
            password_len = 0;
    int8_t conn_flags = 0;

    len += mqttContext_->getProtocolVersion() == MQTT_PROTOCOL_V31 ? 6 : 4;
    /** clientId 长度 */
    if (!mqttContext_->getClientId().empty()) {
        clientId_len = static_cast<int16_t>(mqttContext_->getClientId().size());
    }
    else {
        clientId_len = 20;
        mqttContext_->setClientId(generateRandomString(20));
        STREAM_DEBUG << "MQTT clientId :  " << mqttContext_->getClientId();
    }
    len += clientId_len;

    if (clientId_len == 0) {
        mqttContext_->setCleanSession(1);
    }
    if (mqttContext_->getCleanSession()) {
        conn_flags |= MQTT_CONN_CLEAN_SESSION;
    }
    /** 若设置了MQTT遗嘱 */
    auto willPtr = mqttContext_->getWillPtr();
    if (willPtr && !willPtr->topic.empty()  && !willPtr->payload.empty()) {
        if (willPtr->topicLength && willPtr->payloadLength) {
            conn_flags |= MQTT_CONN_HAS_WILL;
            conn_flags |= ((willPtr->qos & 3) << 3);
            if (willPtr->retain) {
                conn_flags |= MQTT_CONN_WILL_RETAIN;
            }
            len += 2 + willPtr->topicLength;
            len += 2 + willPtr->payloadLength;
        }
    }
    /** 若设置了MQTT用户名 */
    if (!mqttContext_->getUserName().empty()) {
        username_len = static_cast<int16_t>(mqttContext_->getUserName().size());
        if (username_len) {
            conn_flags |= MQTT_CONN_HAS_USERNAME;
            len += 2 + username_len;
        }
    }
    /** 若设置了MQTT用户密码 */
    if (!mqttContext_->getPassword().empty()) {
        password_len = static_cast<int16_t>(mqttContext_->getPassword().size());
        if (password_len) {
            conn_flags |= MQTT_CONN_HAS_PASSWORD;
            len += 2 + password_len;
        }
    }
    /** 造包 */
    mqttContext_->mqttHeadInit();
    MqttContext::MqttHead& head = mqttContext_->getHeader();

    head.type = MQTT_TYPE_CONNECT;
    head.length = len;
    std::cout << "len == " << len << std::endl;

    auto buffer_ = std::make_unique<Buffer>();
    //auto buf = std::unique_ptr<unsigned char>(new unsigned char[bufLength]);
    {
        //int bufLength = mqttEstimateLength(head);
        //std::unique_ptr<char[]> buf = std::make_unique<char[]>(bufLength);
        char buf[5];
        int headLength = mqttContext_->mqttHeadPack(buf); /** 写入MQTT头 */
        std::cout << "head type = " << static_cast<int>(head.type) << "  length =  " << head.length << std::endl;
        //std::cout << "headLength是：" << headLength << " buf[0] = " << static_cast<int>(buf[0])
        //                            << " buf[1] = "<< static_cast<int>(buf[1]) << std::endl;
        buffer_->append(buf,headLength);
    }
    // TODO: Not implement MQTT_PROTOCOL_V5
    if (mqttContext_->getProtocolVersion() == MQTT_PROTOCOL_V31) {
        buffer_->appendInt16(6);
        buffer_->append(MQTT_PROTOCOL_NAME_v31, 6);
    } else {
        buffer_->appendInt16(4);
        buffer_->append(MQTT_PROTOCOL_NAME, 4);
    }
    buffer_->appendInt8(static_cast<int8_t>(mqttContext_->getProtocolVersion()));
    buffer_->appendInt8(static_cast<int8_t>(conn_flags));
    buffer_->appendInt16(static_cast<int16_t>(mqttContext_->getAliveTime()));
    buffer_->appendInt16(static_cast<int16_t>(clientId_len));

    if (clientId_len > 0) {
       buffer_->append(mqttContext_->getClientId());
    }
    if (conn_flags & MQTT_CONN_HAS_WILL) {
        buffer_->appendInt16(static_cast<int16_t>(willPtr->topicLength));
        buffer_->append(willPtr->topic);
        buffer_->appendInt16(willPtr->payloadLength);
        buffer_->append(willPtr->payload);
    }
    if (conn_flags & MQTT_CONN_HAS_USERNAME) {
        buffer_->appendInt16(username_len);
        buffer_->append(mqttContext_->getUserName());
    }
    if (conn_flags & MQTT_CONN_HAS_PASSWORD) {
        buffer_->appendInt16(password_len);
        buffer_->append(mqttContext_->getPassword());
    }
    /**  发送登录信息 */
    send(std::move(buffer_));
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
    mqttContext_->mqttHeadInit();
    auto& head = mqttContext_->getHeader();

    int headLen = 0;
    {
        const char* p = buf.peek();
        headLen = mqttContext_->mqttHeadUnpack(p, static_cast<int>(buf.readableBytes()));
        buf.retrieve(headLen);   /** 跳过头部 */
        assert(headLen > 0);
    }

    switch (head.type) {
        case MQTT_TYPE_CONNACK:
        {
            if (head.length < 2) {
                STREAM_ERROR << "error in MQTT_TYPE_CONNACK ";
                connection_->forceClose();
            }
            int8_t connectAckFlags = buf.readInt8();  /** 连接确认标志 connectAckFlags */
            int8_t connReturnCode = buf.readInt8();   /** 连接返回码 */

            if (connReturnCode != MQTT_CONNACK_ACCEPTED) {
                STREAM_ERROR << "MQTT connect return code = "  << static_cast<int>(connReturnCode);
                connection_->forceClose();
                break;
            }
            isConnected_ = true;  /** MQTT连接成功 */
            if(mqttConnectCallback_){
                mqttConnectCallback_();  /** 执行连接成功回调函数 */
            }

            if (mqttContext_->getAliveTime()) {
                /** 设置应用层心跳, send Ping */
                loop_->runEvery(mqttContext_->getAliveTime(), [this](){
                    sendHeadOnly(MQTT_TYPE_PINGREQ, 0);
                });
            }
        }
            break;
        case MQTT_TYPE_PUBLISH:
        {
            if (head.length < 2) {
                STREAM_ERROR << "error in MQTT_TYPE_PUBLISH ";
                connection_->shutdown();
            }
            /** 重置消息 */
            mqttContext_->mqttMessageInit();
            auto msgPtr = mqttContext_->getMessagePtr();
            msgPtr->topicLength = buf.readInt16();
            msgPtr->topic = buf.retrieveAsString(msgPtr->topicLength);

            if (head.qos > 0) {
                mqttContext_->setMid(buf.readInt16());
            }
            msgPtr->payloadLength = buf.readableBytes();

            msgPtr->payload = buf.retrieveAllAsString();
            msgPtr->qos = mqttContext_->getHeader().qos;
            if (msgPtr->qos == 0) {
                /** 不做任何事 */
                STREAM_DEBUG << "msg->qos == 0";
            }
            else if (msgPtr->qos == 1) {
                sendHeadWithMid(MQTT_TYPE_PUBACK, mqttContext_->getMid());
            }
            else if (msgPtr->qos == 2) {
                sendHeadWithMid(MQTT_TYPE_PUBREC, mqttContext_->getMid());
            }
            if(mqttMessageCallback_) {
                // STREAM_INFO << "running mqttMessageCallback";
                mqttMessageCallback_(msgPtr); /** 执行消息回调 */
            }

        }
            break;
        case MQTT_TYPE_PUBACK:
        case MQTT_TYPE_PUBREC:
        case MQTT_TYPE_PUBREL:
        case MQTT_TYPE_PUBCOMP:
        {
            if (head.length < 2) {
                STREAM_ERROR << "error in MQTT_TYPE_PUBACK ";
                connection_->shutdown();
            }
            mqttContext_->setMid(buf.readInt16());
            if (head.type == MQTT_TYPE_PUBREC) {
                sendHeadWithMid(MQTT_TYPE_PUBREL, mqttContext_->getMid());
            }
            if (head.type == MQTT_TYPE_PUBREL) {
                sendHeadWithMid(MQTT_TYPE_PUBCOMP, mqttContext_->getMid());
            }
        }
            break;
        case MQTT_TYPE_SUBACK:
        {
            if (head.length < 2) {
                STREAM_ERROR << "error in MQTT_TYPE_SUBACK ";
                connection_->shutdown();
            }
            mqttContext_->setMid(buf.readInt16());
            if(mqttSubscribeCallback_){
                mqttSubscribeCallback_();  /** 执行订阅回调 */
            }
        }
            break;
        case MQTT_TYPE_UNSUBACK:
        {
            if (head.length < 2) {
                STREAM_ERROR << "error in MQTT_TYPE_UNSUBACK ";
                connection_->shutdown();
            }
            mqttContext_->setMid(buf.readInt16());
        }
            break;
        case MQTT_TYPE_PINGREQ:
            sendPong();
            break;
        case MQTT_TYPE_PINGRESP:
            //mqttContext_->pingCnt = 0;
            break;
        case MQTT_TYPE_DISCONNECT:
            if (mqttCloseCallback_){
                mqttCloseCallback_(); /** 执行关闭回调 */
            }
            connection_->shutdown();
            break;
        default:
        {
            STREAM_ERROR << "MQTT client received wrong type, the type is : " << mqttContext_->getHeader().type;
        }
            break;
    }
}

/*!
 * \brief 伪随机数算法，生成指定长度的字符串，字符串由26个大写英文字幕组成， 用于clientID */
std::string MqttClient::generateRandomString(int length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dis(65, 90);      /** A ~ Z */
    std::uniform_int_distribution dis_a(97, 121);   /** a ~ z */
    std::uniform_int_distribution dis_num(48, 57);  /** 0 ~ 9 */

    std::string randomString;
    randomString.push_back(static_cast<char>(dis_a(gen)));
    randomString.push_back(static_cast<char>(dis_num(gen)));
    randomString.reserve(length);
    for(int i = 0; i < length - 2; ++i) {
        randomString.push_back(static_cast<char>(dis(gen)));
    }
    return randomString;
}





