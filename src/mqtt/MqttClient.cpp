//
// Created by fzy on 23-7-6.
//

#include "MqttClient.h"

#include "../net/InetAddr.h"


using namespace netflow::net;
using namespace netflow::base;
using namespace std::placeholders;

MqttClient::MqttClient(netflow::net::EventLoop *loop, const netflow::net::InetAddr &serverAddr,
                       const std::string &name)
       : client_(loop, serverAddr, name)
{
    client_.setConnectionCallback(std::bind(&MqttClient::onConnection, this, _1));
    client_.setMessageCallback(std::bind(&MqttClient::onMessage, this, _1, _2, _3));
    client_.enableRetry();
}
/*!
 * \brief 建立TCP长链接 */
void MqttClient::connect() {
    client_.connect();
}

void MqttClient::disconnect() {
    client_.disconnect();
}
/*!
 * \brief 是否已经建立连接
 * \return true: 已经建立连接 */
bool MqttClient::isConnected() {
    return false;
}

/*!
 * \brief MQTT 发布 */
int MqttClient::publish(const std::string &topic, const std::string &payload, int qos, int retain,
                        netflow::net::MqttClient::MqttCallback ackCallback) {
    MqttMessage msg;
    memset(&msg, 0, sizeof msg);
    msg.topic_len = topic.size();
    msg.topic = topic;
    msg.payload_len = payload.size();
    msg.payload = payload;
    msg.qos = qos;   /** 有默认值 */
    msg.retain = retain;
    return publish(msg, std::move(ackCallback));
}
/*!
 * \brief
 *  1. 构建 MQTT header
 *  2. 将header、topic以及mid组包，发出去
 *  3. 发送payload */
int MqttClient::publish(netflow::net::mqtt::MqttMessage& msg, netflow::net::MqttClient::MqttCallback ackCallback) {
    int topic_len = static_cast<int>(msg.topic_len);
    int payload_len = static_cast<int>(msg.payload_len);
    int totalLen = 2 + topic_len + payload_len;
    if (msg.qos > 0)
        totalLen += 2;
    unsigned short mid = 0;


    /** header */
    MqttHead head;
    memset(&head, 0, sizeof head);
    head.type = MQTT_TYPE_PUBLISH;
    head.qos = msg.qos & 3;  /** & 0011: 只会出现 0， 1， 2 三种值 */
    head.retain = msg.retain;
    /** message: topic + payload */
    int bufferLen = mqttEstimateLength(head);
    std::string headerMessage;
    std::string payloadMessage = msg.payload;
    headerMessage = head.type + head.dup + head.qos + head.retain;

    /** send head + topic + mid ??  why not together */
    /** send payload */
    std::unique_lock<std::mutex> lock(mutex_);
    /** 发送 payload*/
    if (connection_) {
        Buffer buffer;
        const char* data = &(*payloadMessage.begin());
        buffer.append(data, payloadMessage.size());
        int32_t len = static_cast<int32_t>(payloadMessage.size());
        int32_t be32 = htonl(len);
        buffer.prepend(&be32, sizeof be32);
        connection_->send(&buffer);
    }

}




int MqttClient::subscribe(const char *topic, int qos, netflow::net::MqttClient::MqttCallback ackCb) {

}


int MqttClient::unSubscribe(const char *topic, netflow::net::MqttClient::MqttCallback ackCb) {


}




void MqttClient::onConnection(const netflow::net::TcpConnectionPtr &conn) {
    if (conn->isConnected()) {

    }

}

void MqttClient::close() {

}

void MqttClient::onMessage(const netflow::net::TcpConnectionPtr &conn, netflow::net::Buffer *buf,
                           base::Timestamp receiveTime) {

}


/*
 * #include <iostream>
#include <sstream>

struct Person {
  std::string name;
  int age;
  std::vector<std::string> hobbies;
};

std::string serializePerson(const Person& person) {
  std::ostringstream oss;
  oss << person.name << "|" << person.age << "|";
  for (const auto& hobby : person.hobbies) {
    oss << hobby << ",";
  }
  return oss.str();
}

Person deserializePerson(const std::string& serializedData) {
  Person person;
  std::istringstream iss(serializedData);
  std::getline(iss, person.name, '|');
  iss >> person.age;
  std::string hobby;
  while (std::getline(iss, hobby, ',')) {
    person.hobbies.push_back(hobby);
  }
  return person;
}

int main() {
  Person person;
  person.name = "John Doe";
  person.age = 30;
  person.hobbies = {"Reading", "Gaming"};

  std::string serializedData = serializePerson(person);
  std::cout << "Serialized data: " << serializedData << std::endl;

  Person deserializedPerson = deserializePerson(serializedData);
  std::cout << "Deserialized person: " << deserializedPerson.name << ", " << deserializedPerson.age << std::endl;
  std::cout << "Hobbies: ";
  for (const auto& hobby : deserializedPerson.hobbies) {
    std::cout << hobby << " ";
  }
  std::cout << std::endl;

  return 0;
}
 * */