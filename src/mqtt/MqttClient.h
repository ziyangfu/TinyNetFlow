//
// Created by fzy on 23-7-6.
//

#ifndef TINYNETFLOW_MQTTCLIENT_H
#define TINYNETFLOW_MQTTCLIENT_H

#include "MqttProtocol.h"

#include "../net/TcpServer.h"

#include <functional>
#include <mutex>
#include <map>

namespace netflow::net {

class MqttClient {
public:
    using MqttCallback = std::function<void(MqttClient*)>;
    using MqttMessageCallback = std::function<void(MqttClient*, MqttMessage*)>;

    MqttClient();
    ~MqttClient();

    void run();

    void stop();

    void setID(const char* id);

    void setWill();

    void setAuth();

    void setPingInterval();

    int lastError();


    void setReconnect();

    void setConnectTimeout();

    int connect();

    int reconnect();

    int disconnect();

    bool isConnected();
    /** 核心方法 */
    int publish();
    int publish(const std::string& topic, const std::string& payload,
                int qos = 0, int retain = 0, MqttCallback ackCb = nullptr);

    int subscribe(const char* topic, int qos = 0, MqttCallback ackCb = nullptr);
    int unSubscribe(const char* topic, MqttCallback ackCb = nullptr);

protected:
    void setAckCallback(int mid, MqttCallback cb);
    /** 请求 */
    void invokeAckCallback(int mid);
    /** onConnection ? */
    static void onMqtt();


private:
    void onConnection(const TcpConnectionPtr& conn);
    void onClose();
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, base::Timestamp receiveTime);

private:
    std::map<int, MqttCallback> ackCallbacks_;
    std::mutex ackCallbacksMutex_;
};
} // namespace netflow::net



#endif //TINYNETFLOW_MQTTCLIENT_H
