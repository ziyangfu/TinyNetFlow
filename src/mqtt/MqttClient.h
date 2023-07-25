//
// Created by fzy on 23-7-6.
//

#ifndef TINYNETFLOW_MQTTCLIENT_H
#define TINYNETFLOW_MQTTCLIENT_H

#include "MqttProtocol.h"

#include "../net/TcpClient.h"

#include <functional>
#include <mutex>
#include <map>
#include <string>

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

        MqttMessage * will;

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


    MqttClient(EventLoop* loop, const InetAddr& serverAddr, const std::string& name = "MQTT Client");
    ~MqttClient();

    void connect();
    int reconnect();
    void disconnect();
    bool isConnected();

    void close();

    /** 核心方法 */
    int publish(MqttMessage& msg, MqttCallback ackCallback = nullptr);
    int publish(const std::string& topic, const std::string& payload,
                int qos = 0, int retain = 0, MqttCallback ackCallback = nullptr);

    int subscribe(const char* topic, int qos = 0, MqttCallback ackCb = nullptr);
    int unSubscribe(const char* topic, MqttCallback ackCb = nullptr);





    void stop();

    void setID(const char* id);

    void setWill();

    void setAuth();

    void setPingInterval();

    int lastError();


    void setReconnect();

    void setConnectTimeout();




protected:
    void setAckCallback(int mid, MqttCallback cb);
    /** 请求 */
    void invokeAckCallback(int mid);


public:
    MqttClientArgs mqttClient_;

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, base::Timestamp receiveTime);

private:
    TcpClient client_;
    TcpConnectionPtr connection_;
    std::map<int, MqttCallback> ackCallbacks_;
    std::mutex mutex_;

    uint8_t version;

private:






};

} // namespace netflow::net



#endif //TINYNETFLOW_MQTTCLIENT_H
