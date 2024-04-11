//
// Created by fzy on 23-8-16.
//

#ifndef TINYNETFLOW_MQTTCONTEXT_H
#define TINYNETFLOW_MQTTCONTEXT_H

#include <string>
#include <memory>

#include "src/mqtt/MqttProtocol.h"

namespace netflow::net::mqtt {
class MqttContext {
public:
    struct MqttHead {
        MqttHead()
            : type(0),
              dup(0),
              qos(0),
              retain(0),
              length(0)
        {
        }
        int8_t type;       /** MQTT 控制报文类型 */
        int8_t dup;        /** 重发标志 */
        int8_t qos;        /** PUBLISH报文的服务质量等级 */
        int8_t retain;     /** 保留标志 */
        int  length;       /** 剩余长度， 当前报文剩余部分的字节数，包括可变报头和负载的数据 */
    };
    struct MqttMessage {
        MqttMessage()
            : topicLength(0),
              topic(),
              payloadLength(0),
              payload(),
              qos(0),
              retain(0)
        {
        }
        void swap(MqttMessage& that) {
            std::swap(topicLength, that.topicLength);
            std::swap(topic, that.topic);
            std::swap(payloadLength, that.payloadLength);
            std::swap(payload, that.payload);
            std::swap(qos, that.qos);
            std::swap(retain, that.retain);
        }
        int16_t         topicLength;
        std::string     topic;
        int16_t         payloadLength;
        std::string     payload;
        int8_t          qos;
        int8_t          retain;
    };

    MqttContext();
    ~MqttContext();
    bool mqttHeadInit() {
        head_.type      = 0;
        head_.dup       = 0;
        head_.qos       = 0;
        head_.retain    = 0;
        head_.length    = 0;
    }
    bool mqttMessageInit() {
        MqttMessage that;
        message_->swap(that);
    }
    bool mqttWillInit() {
        MqttMessage that;
        will_->swap(that);
    }
    int mqttHeadPack(char* buf);
    int mqttHeadUnpack(const char* buf, int len);
    int mqttEstimateLength() const {
        /** header + 剩余长度（max 4 byte） + length */
        return 1 + 4 + static_cast<int>(head_.length);
    }

    void setProtocolVersion(int8_t version) { protocolVersion_ = version; }
    int8_t getProtocolVersion() const { return protocolVersion_; }

    void setAliveTime(int16_t second) {  keepAlive_ = second;}
    int16_t getAliveTime() const { return keepAlive_; }

    void setClientId(const std::string& id) { clientId_ = id; }
    const std::string& getClientId() const { return clientId_; }

    void setUserName(const std::string& name) { userName_ = name; }
    const std::string& getUserName() const { return userName_; }

    void setPassword(const std::string& pwd) { password_ = pwd; }
    const std::string& getPassword() const { return password_; }

    void setCleanSession(int8_t num) { cleanSession = num; }
    int8_t getCleanSession() const { return cleanSession; }

    void setMid(int16_t id) { mid_ = id; }
    int16_t getMid() const { return mid_; }

    auto getMessagePtr() { return message_; }
    auto getWillPtr() { return will_; }
    void setWill(MqttMessage& msg) { will_->swap(msg);}
    MqttHead& getHeader() { return head_; }

    std::string printMqttContextToString();

private:
    int8_t protocolVersion_;
    int16_t keepAlive_;   /** 秒 */
    MqttHead head_;
    std::shared_ptr<MqttMessage> message_;
    std::shared_ptr<MqttMessage> will_;
    std::string clientId_;  /** 客户端标识符 */
    /** 认证 */
    std::string userName_;
    std::string password_;
    int8_t cleanSession;
    int16_t mid_;   // for MQTT_TYPE_SUBACK, MQTT_TYPE_PUBACK

};
}  // namespace netflow::net::mqtt

#endif //TINYNETFLOW_MQTTCONTEXT_H
