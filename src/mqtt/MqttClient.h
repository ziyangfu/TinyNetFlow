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
#include "src/net/TcpClient.h"


namespace netflow::net {

using namespace mqtt;

class Buffer;

struct MqttClientArgs {
public:
    int connectTimeout;   // 计划删除
    // + reconnect
    unsigned char protocolVersion;   /** TODO（fzy）： 替换为 uint8_t */
    unsigned char cleanSession: 1;   /** TODO 不使用位域，没必要 */
    unsigned char ssl: 1;
    unsigned char allocedSslCtx: 1;
    unsigned char connected: 1;
    unsigned short keepAlive;
    int pingCnt;
    std::string clientId;
    MqttMessage* will;
    /** auth */
    std::string userName;
    std::string password;
    MqttHead head;
    int error; // for MQTT_TYPE_CONNACK
    int mid;   // for MQTT_TYPE_SUBACK, MQTT_TYPE_PUBACK
    MqttMessage message;
    void* userdata;
};

class MqttClient {
public:
    using MqttCallback = std::function<void()>;
    using MqttMessageCallback = std::function<void (const std::string& message)>;

    MqttClient(EventLoop* loop, const InetAddr& serverAddr, const std::string& name = "MqttClient");
    ~MqttClient();

    int connect();
    int reconnect();
    void disconnect();
    bool isConnected() const { return isConnected_; }
    void stop();
    void close();

    /** 核心方法 */
    int publish(MqttMessage& msg, MqttCallback ackCallback = nullptr);
    int publish(const std::string& topic, const std::string& payload,
                int qos = 0, int retain = 0, MqttCallback ackCallback = nullptr);

    int subscribe(const char* topic, int qos = 0);
    int unSubscribe(const char* topic, MqttCallback ackCallback = nullptr);

    void setID(const std::string& id) {
        mqttClientArgs_->clientId = id;
    }

    void setWill(MqttMessage* message) {
        mqttClientArgs_->will = message;
    }

    void setAuth(std::string& username, std::string& password) {
        mqttClientArgs_->userName = username;
        mqttClientArgs_->password = password;
    }

    void setPingInterval(int sec) {
        mqttClientArgs_->keepAlive = sec;
    }

    int setSslCtx() {
        // pass
    }
    void setReconnect() {
        // pass
    }

    int getLastError() const {
        return mqttClientArgs_->error;
    }



    void setConnectTimeout(int ms) {
        mqttClientArgs_->connectTimeout = ms;
    }

    void setMqttMessageCallback(MqttMessageCallback cb) { mqttMessageCallback_ = std::move(cb);};
    void setMqttConnectCallback(MqttCallback cb) { mqttConnectCallback_ = std::move(cb); }
    void setMqttCloseCallback(MqttCallback cb) { mqttCloseCallback_ = std::move(cb); }
    void setMqttSubscribeCallback(MqttCallback cb) { mqttSubscribeCallback_ = std::move(cb); }
    void setMqttPublishCallback(MqttCallback cb) { mqttPublishCallback_ = std::move(cb); }
private:
    int mqttClientLogin();
    void send(std::string& message);
    void send(std::unique_ptr<Buffer> buffer, const int len);
    int sendHeadOnly(int type, int length);
    int sendHeadWithMid(int type, unsigned short mid);
    int sendPong();
    void sendPing();
    void onConnection(const TcpConnectionPtr& conn);
    /** MQTT协议解析 */
    void onMessage(const TcpConnectionPtr&, Buffer& buf, Timestamp receiveTime);
    std::string& mqttProtocolParse(Buffer& buf);
    int16_t mqttNextMid();
    void setHeartbeat(int intervalMs);

private:
    MqttMessageCallback mqttMessageCallback_;
    MqttCallback        mqttConnectCallback_;
    MqttCallback        mqttCloseCallback_;
    MqttCallback        mqttSubscribeCallback_;
    MqttCallback        mqttPublishCallback_;

    TcpClient client_;
    TcpConnectionPtr connection_;
    MqttHeaderCodec mqttHeaderCodec_;
    std::mutex mutex_;
    std::atomic_bool isConnected_;
    std::unique_ptr<MqttClientArgs> mqttClientArgs_;



    uint8_t version;
    std::map<int, MqttCallback> ackCallbacks_;
};

} // namespace netflow::net



#endif //TINYNETFLOW_MQTTCLIENT_H
