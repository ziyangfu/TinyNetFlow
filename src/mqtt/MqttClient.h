//
// Created by fzy on 23-7-6.
//

#ifndef TINYNETFLOW_MQTTCLIENT_H
#define TINYNETFLOW_MQTTCLIENT_H

#include "MqttProtocol.h"

#include "../net/TcpClient.h"
#include "MqttHeaderCodec.h"

#include <functional>
#include <mutex>
#include <map>
#include <string>
#include <memory>
#include <string_view>

namespace netflow::net {

using namespace mqtt;

class Buffer;

class MqttClient {
public:
    struct MqttClientArgs {
    public:
        std::string host;
        uint16_t port;
        int connectTimeout;
        // + reconnect
        unsigned char protocolVersion;
        unsigned char cleanSession: 1;
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

        using MqttClientCallabck = std::function<void(MqttClientArgs* cli, int type)>;
        MqttClientCallabck cb;

        void* userdata;
    private:
        EventLoop* loop_;
        std::mutex mqttClientArgsMutex_;
    };

    using MqttCallback = std::function<void(MqttClient*)>;
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

    int subscribe(const char* topic, int qos = 0, MqttCallback ackCallback = nullptr);
    int unSubscribe(const char* topic, MqttCallback ackCallback = nullptr);







    void setID(const char* id);

    void setWill();

    void setAuth();

    void setPingInterval(int value);

    int lastError();


    void setReconnect();

    void setConnectTimeout();



    void setMqttMessageCallback(MqttMessageCallback cb) { mqttMessageCallback_ = std::move(cb);};
    void setMqttConnectCallback() {}
    void setMqttCloseCallback() {}

protected:
    void setAckCallback(int mid, MqttCallback cb);
    /** 请求 */
    void invokeAckCallback(int mid);


public:
    std::unique_ptr<MqttClientArgs> mqttClientArgs_;

private:
    int mqttClientLogin();
    void send(std::string& message);
    void send(std::unique_ptr<Buffer> buffer, const int len);
    int sendHeadOnly(int type, int length);
    int sendHeadWithMid(int type, unsigned short mid);
    int sendPong();
    void onConnection(const TcpConnectionPtr& conn);
    /** MQTT协议解析 */
    void onMessage(const TcpConnectionPtr&, Buffer& buf, Timestamp receiveTime);
    std::string& mqttProtocolParse(Buffer& buf);
    int16_t mqttNextMid();

private:

    MqttMessageCallback mqttMessageCallback_;

    TcpClient client_;
    TcpConnectionPtr connection_;
    MqttHeaderCodec mqttHeaderCodec_;

    std::mutex mutex_;
    std::atomic_bool isConnected_;

    uint8_t version;
    std::map<int, MqttCallback> ackCallbacks_;

};

} // namespace netflow::net



#endif //TINYNETFLOW_MQTTCLIENT_H
