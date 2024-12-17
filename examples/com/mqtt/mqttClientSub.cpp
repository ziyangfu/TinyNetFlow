//
// Created by fzy on 23-8-8.
//

#include "src/mqtt/MqttClient.h"
#include "netflow/net//InetAddr.h"
#include "netflow/net//EventLoopThread.h"
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
    Logger::get().set_level(spdlog::level::info);
    bool running = true;
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
    client.setMqttConnectCallback( [&client, topic]() mutable {
        std::cout << "connected" << std::endl;
        client.subscribe(topic);
    });
    client.setMqttMessageCallback([&client](const std::shared_ptr<MqttContext::MqttMessage> msg){
        cout << "received topic: " << msg->topic << endl;
        cout << "received payload: " << msg->payload << endl;
    });
    client.setMqttCloseCallback([&running]() mutable {
        std::cout << "disconnected" << std::endl;
        running = false;
    });

    client.setPingInterval(10);
    client.connect();
    while(running){
        std::this_thread::yield();
    }
    return 0;
}