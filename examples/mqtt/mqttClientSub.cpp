//
// Created by fzy on 23-8-8.
//

#include "src/mqtt/MqttClient.h"
#include "src/net/InetAddr.h"
#include "src/net/EventLoopThread.h"
#include "src/base/Logging.h"

#include <iostream>
#include <thread>
#include <chrono>

using namespace netflow::net::mqtt;
using namespace std;

const bool  TEST_MQTT_SSL = false;
const bool TEST_MQTT_AUTH = false;
const bool TEST_MQTT_RECONNECT = false;
const int TEST_MQTT_QOS = 0;

int main(int argc, char** argv) {
    Logger::get().set_level(spdlog::level::trace);
    if (argc < 4) {
        cout << "Usage : "<< argv[0] << " host port topic" << endl;
        return -10;
    }
    EventLoopThread loopThread;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddr addr_(argv[1], port);
    MqttClient client(loopThread.startLoop(), addr_);

    std::string topic = argv[3];

    /** 三个回调函数 */
    client.setMqttConnectCallback( [&client, topic](){
        std::cout << "connected" << std::endl;
        client.subscribe(topic.c_str());
    });
    client.setMqttMessageCallback([&client](const MqttMessage* msg){
        printf("topic: %.*s\n", msg->topic_len, msg->topic);
        printf("payload: %.*s\n", msg->payload_len, msg->payload);
    });
    client.setMqttCloseCallback([](){
        std::cout << "disconnected" << std::endl;
    });

    client.setPingInterval(10);
    client.connect();
    while(1){
        std::this_thread::yield();
    }
    return 0;
}