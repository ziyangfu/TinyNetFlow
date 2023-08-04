//
// Created by fzy on 23-8-4.
//
#include "../../src/mqtt/MqttClient.h"
#include "../../src/net/InetAddr.h"
#include "../../src/net/EventLoopThread.h"

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
    if (argc < 5) {
        cout << "Usage : "<< argv[0] << " host port topic payload" << endl;
        return -10;
    }
    EventLoopThread loopThread;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddr addr_(argv[1], port);
    MqttClient client(loopThread.startLoop(), addr_);

    /** 三个回调函数 */
    client.setMqttMessageCallback([](const std::string& message){

    });
    client.setMqttConnectCallback();
    client.setMqttCloseCallback();

    client.setPingInterval(10);


    client.connect();
    this_thread::sleep_for(chrono::seconds(5));
    return 0;
}