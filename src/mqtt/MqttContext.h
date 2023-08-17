//
// Created by fzy on 23-8-16.
//

#ifndef TINYNETFLOW_MQTTCONTEXT_H
#define TINYNETFLOW_MQTTCONTEXT_H

#if 1

#include <string>
#include <memory>

#include "src/mqtt/MqttProtocol.h"

namespace netflow::net::mqtt {
    class MqttContext {
    public:
        MqttContext();
        ~MqttContext();
        void setProtocolVersion(unsigned char version) { protocolVersion_ = version; }
        unsigned char getProtocolVersion() const { return protocolVersion_; }
        void setAliveTime(unsigned short second) {  keepAlive_ = second;}
        unsigned short getAliveTime() const { return keepAlive_; }

        void setClientId(const std::string& id) { clientId_ = id; }
        std::string getClientId() const { return clientId_; }
        void setWill(MqttMessage* msg) { will_ = msg; }
        MqttMessage* getWill() const { return will_; }

        void setUserName(const std::string& name) { userName_ = name; }
        const std::string& getUserName() const { return userName_; }
        void setPassword(const std::string& pwd) { password_ = pwd; }
        const std::string& getPassword() const { return password_; }

        void setCleanSession(unsigned char num) { cleanSession = num; }
        unsigned char getCleanSession() const { return cleanSession; }

        MqttHead& getHeader() { return head_; }
        void setHeader(MqttHead header) { head_ = header; }

        void setMessage(MqttMessage* msg) { message_ = msg; }
        MqttMessage* getMessage() { return message_;}

        void setMid(int id) { mid_ = id; }
        int getMid() const { return mid_; }

        void swap(MqttContext context);

        std::string printMqttContextToString();
    private:
        unsigned char protocolVersion_;
        unsigned short keepAlive_;   /** 秒 */
        MqttHead head_;
        //std::unique_ptr<MqttMessage> message_;
        MqttMessage* message_;
        std::string clientId_;  /** 客户端标识符 */
        //std::unique_ptr<MqttMessage> will_;
        MqttMessage* will_;
        /** auth */
        std::string userName_;
        std::string password_;

        unsigned char cleanSession;
        int error_; // for MQTT_TYPE_CONNACK
        int mid_;   // for MQTT_TYPE_SUBACK, MQTT_TYPE_PUBACK
    };
}  // namespace netflow::net::mqtt

#endif

#endif //TINYNETFLOW_MQTTCONTEXT_H
