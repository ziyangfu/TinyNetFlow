//
// Created by fzy on 23-8-4.
//
/** usage:
 *          pub:  ./mqttClientPub localhost 1883 OTA allow_to_update
 *          sub:  ./mqttClientSub localhost 1883 OTA request_update
 *          proxy: mosquite
 *          */
#include "src/mqtt/MqttClient.h"
#include "src/net/InetAddr.h"
#include "src/net/EventLoopThread.h"

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

    std::string topic = argv[3];
    std::string payload = argv[4];

    /** 三个回调函数 */
    client.setMqttConnectCallback( [&client, topic, payload](){
        std::cout << "connected" << std::endl;
        client.subscribe(topic.c_str());
    });
    client.setMqttMessageCallback([&client](const MqttMessage& message){
        std::cout << "topic : " << message.topic << std::endl;
        std::cout << "payload : " << message.payload << std::endl;
    });
    client.setMqttCloseCallback([](){
        std::cout << "disconnected" << std::endl;
    });

    client.setPingInterval(10);
    client.connect();
    // this_thread::sleep_for(chrono::seconds(5));
    return 0;
}