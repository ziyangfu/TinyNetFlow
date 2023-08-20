//
// Created by fzy on 23-8-16.
//

#ifndef TINYNETFLOW_MQTTCONTEXT_H
#define TINYNETFLOW_MQTTCONTEXT_H

#if 1

#include <string>
#include <memory>

//#include "src/mqtt/MqttProtocol.h"

namespace netflow::net::mqtt {
class MqttContext {
public:
    MqttContext();
    ~MqttContext();
public:
    /** 位域并没有节省空间，8字节，pending 3字节 */
    struct MqttHead {
        int8_t type;   /** MQTT 控制报文类型 */
        int8_t dup;   /** 重发标志 */
        int8_t qos;   /** PUBLISH报文的服务质量等级 */
        int8_t retain;   /** 保留标志 */
        int8_t  length;        /** 剩余长度， 当前报文剩余部分的字节数，包括可变报头和负载的数据 */
    };

    struct MqttMessage {
        int16_t         topic_len;
        std::string     topic;
        int16_t         payload_len;
        std::string     payload;
        int8_t          qos;
        int8_t          retain;
    };

    void swap(MqttContext context);

    void init();

    bool mqttHeadInit();
    bool mqttMessageInit();
    bool mqttWillInit();
    int mqttHeadPack();
    int mqttHeadUnpack();

    void setProtocolVersion(unsigned char version) { protocolVersion_ = version; }
    unsigned char getProtocolVersion() const { return protocolVersion_; }
    void setAliveTime(unsigned short second) {  keepAlive_ = second;}
    unsigned short getAliveTime() const { return keepAlive_; }

    void setClientId(const std::string& id) { clientId_ = id; }
    std::string getClientId() const { return clientId_; }

    void setWill() { }
    MqttMessage* getWill() const {}
    void setMessage() {}
    MqttMessage* getMessage() {}

    void setUserName(const std::string& name) { userName_ = name; }
    const std::string& getUserName() const { return userName_; }
    void setPassword(const std::string& pwd) { password_ = pwd; }
    const std::string& getPassword() const { return password_; }

    void setCleanSession(int8_t num) { cleanSession = num; }
    unsigned char getCleanSession() const { return cleanSession; }

    MqttHead& getHeader() { return head_; }
    void setHeader(MqttHead header) { head_ = header; }

    void setMid(int16_t id) { mid_ = id; }
    int16_t getMid() const { return mid_; }

    std::string printMqttContextToString();
private:
    int8_t protocolVersion_;
    int16_t keepAlive_;   /** 秒 */
    MqttHead head_;
    std::unique_ptr<MqttMessage> message_;
    std::unique_ptr<MqttMessage> will_;
    std::string clientId_;  /** 客户端标识符 */
    /** auth */
    std::string userName_;
    std::string password_;

    int8_t cleanSession;
    int16_t mid_;   // for MQTT_TYPE_SUBACK, MQTT_TYPE_PUBACK
};
}  // namespace netflow::net::mqtt

#endif

#endif //TINYNETFLOW_MQTTCONTEXT_H
