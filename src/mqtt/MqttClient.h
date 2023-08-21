//
// Created by fzy on 23-7-6.
//

#ifndef TINYNETFLOW_MQTTCLIENT_H
#define TINYNETFLOW_MQTTCLIENT_H

#include <functional>
#include <mutex>
#include <map>
#include <string>
#include <memory>
#include <string_view>

#include "src/mqtt/MqttProtocol.h"
#include "src/mqtt/MqttHeaderCodec.h"
#include "src/mqtt/MqttContext.h"
#include "src/net/TcpClient.h"
#include <src/net/EventLoop.h>


namespace netflow::net {

using namespace mqtt;

class Buffer;

class MqttClient {
public:
    using MqttCallback = std::function<void()>;
    using MqttMessageCallback = std::function<void (const std::shared_ptr<MqttContext::MqttMessage> message)>;

    MqttClient(EventLoop* loop, const InetAddr& serverAddr, const std::string& name = "MqttClient");
    ~MqttClient() = default;

    int connect();
    int reconnect();
    void disconnect();
    bool isConnected() const { return isConnected_; }
    void stop();
    void close();

    /** 核心方法 */
    int publish(std::shared_ptr<MqttContext::MqttMessage> msg);
    int publish(const std::string& topic, const std::string& payload,
                int8_t qos = 0, int8_t retain = 0);
    int publish(const char* topic, const char* payload, int8_t qos = 0, int8_t retain = 0);

    int subscribe(const char* topic, int qos = 0);
    int subscribe(const std::string& topic);
    int unSubscribe(const char* topic);
    int unSubscribe(const std::string& topic);

    void setID(const std::string& id) {
        mqttContext_->setClientId(id);
    }

    void setWill(MqttContext::MqttMessage& message) {
        mqttContext_->setWill(message);
    }

    void setAuth(std::string& username, std::string& password) {
        mqttContext_->setUserName(username);
        mqttContext_->setPassword(password);
    }

    void setPingInterval(int16_t sec) {
        mqttContext_->setAliveTime(sec);
    }

    void setReconnect() {
        // pass
    }

    void setMqttMessageCallback(MqttMessageCallback cb) { mqttMessageCallback_ = std::move(cb);}
    void setMqttConnectCallback(MqttCallback cb) { mqttConnectCallback_ = std::move(cb); }
    void setMqttCloseCallback(MqttCallback cb) { mqttCloseCallback_ = std::move(cb); }
    void setMqttSubscribeCallback(MqttCallback cb) { mqttSubscribeCallback_ = std::move(cb); }
    void setMqttPublishCallback(MqttCallback cb) { mqttPublishCallback_ = std::move(cb); }
private:
    int mqttClientLogin();
    void send(std::string& message);
    void send(const char* message, int length);
    void send(std::unique_ptr<Buffer> buffer);
    int sendHeadOnly(int8_t type, int length);
    int sendHeadWithMid(int8_t type, int16_t mid);
    int sendPong();
    void sendPing();
    void onConnection(const TcpConnectionPtr& conn);
    /** MQTT协议解析 */
    void onMessage(const TcpConnectionPtr&, Buffer& buf, Timestamp receiveTime);
    std::string& mqttProtocolParse(Buffer& buf);
    int16_t mqttNextMid();
    std::string generateRandomString(int length);

private:
    MqttMessageCallback mqttMessageCallback_;
    MqttCallback        mqttConnectCallback_;
    MqttCallback        mqttCloseCallback_;
    MqttCallback        mqttSubscribeCallback_;
    MqttCallback        mqttPublishCallback_;
    EventLoop* loop_;
    TcpClient client_;
    TcpConnectionPtr connection_;
    MqttHeaderCodec mqttHeaderCodec_;
    std::mutex mutex_;
    std::atomic_bool isConnected_;
    std::unique_ptr<MqttContext> mqttContext_;

    int8_t version;
    //std::map<int, MqttCallback> ackCallbacks_;
};

} // namespace netflow::net



#endif //TINYNETFLOW_MQTTCLIENT_H
