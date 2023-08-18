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

/*****************************************************************************************************/
int hv_rand(int min, int max) {
    static int s_seed = 0;
    assert(max > min);

    if (s_seed == 0) {
        s_seed = time(NULL);
        srand(s_seed);
    }

    int _rand = rand();
    _rand = min + (int) ((double) ((double) (max) - (min) + 1.0) * ((_rand) / ((RAND_MAX) + 1.0)));
    return _rand;
}

char* hv_random_string(char *buf, int len) {
    static char s_characters[] = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',
            'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
            'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    };
    int i = 0;
    for (; i < len; i++) {
        buf[i] = s_characters[hv_rand(0, sizeof(s_characters) - 1)];
    }
    buf[i] = '\0';
    return buf;
}
/*****************************************************************************************************/


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
int MqttClient::publish(const std::string &topic, const std::string &payload, int qos, int retain) {
    publish(topic.c_str(), payload.c_str());
}

int MqttClient::publish(const char *topic, const char *payload, int qos, int retain) {
    MqttMessage msg;
    memset(&msg, 0, sizeof msg);
    msg.topic_len = strlen(topic);
    msg.topic = topic;
    msg.payload_len = strlen(payload);
    msg.payload = payload;
    msg.qos = qos;   /** 有默认值 */
    msg.retain = retain;
    return publish(msg);
}
/*!
 * \brief
 *  1. 构建 MQTT header
 *  2. 将header、topic以及mid组包，发出去
 *  3. 发送payload */
int MqttClient::publish(netflow::net::mqtt::MqttMessage& msg) {
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
    buffer_->append(msg.topic, topic_len);
    if (msg.qos) {
        mid = mqttNextMid();
        buffer_->appendInt16(mid);
        totalLength += 2;
    }
    /** send: head + topic + mid */
    totalLength += headLength + 2 + topic_len;
    send(std::move(buffer_));
    /** send:payload */
    send(msg.payload, payload_len);
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
    send(std::move(buffer_));
}

int MqttClient::unSubscribe(const char *topic) {
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
    send(std::move(buffer_));
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
    std::unique_lock<std::mutex> lock(mutex_);
    if (connection_) {
        mqttHeaderCodec_.send(connection_.get(), std::move(buffer));
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
    send(std::move(buffer_));
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
    unsigned short  clientId_len = 0,
                    will_topic_len = 0,
                    will_payload_len = 0,
                    username_len = 0,
                    password_len = 0;
    unsigned char conn_flags = 0;

    char clientID[64];


    len += mqttContext_->getProtocolVersion() == MQTT_PROTOCOL_V31 ? 6 : 4;
    /** clientId 长度 */
    if (!mqttContext_->getClientId().empty()) {
        clientId_len = static_cast<unsigned short>(mqttContext_->getClientId().size());
    }
    else {
        clientId_len = 20;
        //mqttContext_->setClientId(generateRandomString(20));
        hv_random_string(clientID, clientId_len);
        STREAM_INFO << "MQTT clientId :  " << clientID;
    }
    len += clientId_len;

    if (clientId_len == 0) { mqttContext_->setCleanSession(1); }
    if (mqttContext_->getCleanSession()) {
        conn_flags |= MQTT_CONN_CLEAN_SESSION;
    }

    if (mqttContext_->getWill() && !mqttContext_->getWill()->topic
                                && !mqttContext_->getWill()->payload) {
        will_topic_len = mqttContext_->getWill()->topic_len ?
                mqttContext_->getWill()->topic_len :
                static_cast<unsigned short>(strlen(mqttContext_->getWill()->topic));
        will_payload_len = mqttContext_->getWill()->payload_len ?
                mqttContext_->getWill()->payload_len :
                static_cast<unsigned short>(strlen(mqttContext_->getWill()->payload));
        if (will_topic_len && will_payload_len) {
            conn_flags |= MQTT_CONN_HAS_WILL;
            conn_flags |= ((mqttContext_->getWill()->qos & 3) << 3);
            if (mqttContext_->getWill()->retain) {
                conn_flags |= MQTT_CONN_WILL_RETAIN;
            }
            len += 2 + will_topic_len;
            len += 2 + will_payload_len;
        }
    }
    if (!mqttContext_->getUserName().empty()) {
        username_len = static_cast<unsigned short>(mqttContext_->getUserName().size());
        if (username_len) {
            conn_flags |= MQTT_CONN_HAS_USERNAME;
            len += 2 + username_len;
        }
    }
    if (!mqttContext_->getPassword().empty()) {
        password_len = static_cast<unsigned short>(mqttContext_->getPassword().size());
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

    auto buffer_ = std::make_unique<Buffer>();
    //auto buf = std::unique_ptr<unsigned char>(new unsigned char[bufLength]);
    {
        //int bufLength = mqttEstimateLength(head);
        //std::unique_ptr<char[]> buf = std::make_unique<char[]>(bufLength);
        char buf[5];
        int headLength = mqttHeadPack(&head, buf); /** 写入MQTT头 */
        //std::cout << "head type = " << static_cast<int>(head.type) << "  length =  " << head.length << std::endl;
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
       // buffer_->append(mqttContext_->getClientId().c_str(), clientId_len);
       buffer_->append(clientID, clientId_len);
    }
    if (conn_flags & MQTT_CONN_HAS_WILL) {
        buffer_->appendInt16(static_cast<int16_t>(will_payload_len));
        buffer_->append(mqttContext_->getWill()->topic, will_topic_len);
        buffer_->appendInt16(static_cast<int16_t>(will_topic_len));
        buffer_->append(mqttContext_->getWill()->payload, will_payload_len);
    }
    if (conn_flags & MQTT_CONN_HAS_USERNAME) {
        buffer_->appendInt16(static_cast<int16_t>(username_len));
        buffer_->append(mqttContext_->getUserName().c_str(), username_len);
    }
    if (conn_flags & MQTT_CONN_HAS_PASSWORD) {
        buffer_->appendInt16(static_cast<int16_t>(password_len));
        buffer_->append(mqttContext_->getPassword().c_str(), password_len);
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
    STREAM_INFO << "buffer size = " << buf.readableBytes();
    MqttHead head = mqttContext_->getHeader();
    memset(&head, 0, sizeof(head));
    int headLen = 0;
    {
        const char* p = buf.peek();
        headLen = mqttHeadUnpack(&head, p, static_cast<int>(buf.readableBytes()));
        buf.retrieve(headLen);   /** 跳过头部 */
        STREAM_INFO << "head len = " << headLen;
        assert(headLen > 0);
    }

    switch (head.type) {
        case MQTT_TYPE_CONNACK:
        {
            if (head.length < 2) {
                STREAM_ERROR << "error in MQTT_TYPE_CONNACK ";
                connection_->forceClose();
            }
            int8_t conn_flags = buf.readInt8();  /** 连接确认标志 connectAckFlags */
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
                /** 设置应用层心跳 */
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
            STREAM_INFO << "执行到了 MQTT_TYPE_PUBLISH ";
            MqttMessage msg;
            memset(&msg, 0, sizeof(msg));
            STREAM_INFO << "执行到了 MQTT_TYPE_PUBLISH ";
            msg.topic_len = buf.readInt16();
            STREAM_INFO << "msg->topic_len = " << msg.topic_len;

            std::string topic = buf.retrieveAsString(msg.topic_len);
            msg.topic = topic.c_str();

            if (head.qos > 0) {
                mqttContext_->setMid(buf.readInt16());
            }
            msg.payload_len = buf.readableBytes();
            STREAM_INFO << "msg->payload_len = " << msg.payload_len;

            std::string payload = buf.retrieveAllAsString();
            msg.payload = payload.c_str();
            msg.qos = mqttContext_->getHeader().qos;
            if (msg.qos == 0) {
                /** 不做任何事 */
                STREAM_TRACE << "msg->qos == 0";
            }
            else if (msg.qos == 1) {
                sendHeadWithMid(MQTT_TYPE_PUBACK, mqttContext_->getMid());
            }
            else if (msg.qos == 2) {
                sendHeadWithMid(MQTT_TYPE_PUBREC, mqttContext_->getMid());
            }
            if(mqttMessageCallback_) {
                STREAM_INFO << "mqttMessageCallback_";
                mqttMessageCallback_(&msg); /** 执行消息回调 */
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
    std::uniform_int_distribution dis(65, 90);  /** A ~ Z */
    std::uniform_int_distribution dis_a(97, 121); /** a ~ z */
    std::uniform_int_distribution dis_num(48, 57);

    std::string randomString;
    randomString.push_back(static_cast<char>(dis_a(gen)));
    randomString.push_back(static_cast<char>(dis_num(gen)));
    randomString.reserve(length);
    for(int i = 0; i < length - 2; ++i) {
        randomString.push_back(static_cast<char>(dis(gen)));
    }
    return randomString;
}





