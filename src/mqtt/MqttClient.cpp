//
// Created by fzy on 23-7-6.
//

#include "MqttClient.h"

#include "../base/Logging.h"

#include "../net/InetAddr.h"


using namespace netflow::net;
using namespace netflow::base;
using namespace std::placeholders;

MqttClient::MqttClient(netflow::net::EventLoop *loop, const netflow::net::InetAddr &serverAddr,
                       const std::string &name)
       : client_(loop, serverAddr, name),  /** 建立 TCP 连接 */
         isConnected_(false),
         mqttClientArgs_(std::make_unique<MqttClientArgs>()),
         mqttHeaderCodec_(std::bind(&MqttClient::onMessage, this, _1, _2, _3))
{
    client_.setConnectionCallback(std::bind(&MqttClient::onConnection, this, _1));
    client_.setMessageCallback(std::bind(&MqttHeaderCodec::onMessage, mqttHeaderCodec_, _1, _2, _3 ));
}
/*!
 * \brief 建立TCP长链接 */
int MqttClient::connect(const std::string& host, int port, int ssl) {
    if (!mqttClientArgs_) {
        STREAM_ERROR << "mqttClientArgs_ ERROR!";
        return -1;
    }
    mqttClientArgs_->host = host;
    mqttClientArgs_->port = port;
    mqttClientArgs_->ssl = ssl;
    if (ssl) {}
    if (mqttClientArgs_->connectTimeout > 0) {}
    /** 以上都不需要，在构造函数中进行，建立TCP连接 */



}

int MqttClient::reconnect() {

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




int MqttClient::subscribe(const char *topic, int qos, netflow::net::MqttClient::MqttCallback ackCallback) {

    int topic_len = strlen(topic);
    int len = 2 + 2 + topic_len + 1;

    MqttHead head;
    memset(&head, 0, sizeof head);
    head.type = MQTT_TYPE_SUBSCRIBE;
    head.qos = 1;
    head.length = len;

    int buflen = mqttEstimateLength(head);
    /** 构建一条message : head + mid + topic + qos */
    std::string message;

    /**  发送  */
    std::unique_lock<std::mutex> lock(mutex_);
    if (connection_) {
        send(message);
    }
    lock.unlock();

}


int MqttClient::unSubscribe(const char *topic, netflow::net::MqttClient::MqttCallback ackCallback) {


}

/*!
 * \brief 发送消息
 * \private */
void MqttClient::send(std::string &message) {
    Buffer buffer;
    connection_->send(&buffer);
}
/*!
 * \brief TCP连接建立后的回调函数，发送登录验证信息 */
void MqttClient::onConnection(const netflow::net::TcpConnectionPtr &conn) {

    /** 在这里要发送一条MQTT登录消息 */
    if (conn->isConnected()) {

    }

}

void MqttClient::close() {

}

void MqttClient::onMessage(const TcpConnectionPtr&, const std::string& message, Timestamp receiveTime) {

}

/*!
 * MQTT_TYPE_CONNECT
 * 2 + protocol_name + 1 protocol_version + 1 conn_flags + 2 keepalive + 2 + [client_id] +
 * [2 + will_topic + 2 + will_payload] +
 * [2 + username] + [2 + password]
 */
int MqttClient::mqttClientLogin() {
    int len = 2 + 1 + 1 + 2 + 2;
    unsigned short  cid_len = 0,
                    will_topic_len = 0,
                    will_payload_len = 0,
                    username_len = 0,
                    password_len = 0;
    unsigned char conn_flags = 0;
    len += mqttClientArgs_->protocolVersion = MQTT_PROTOCOL_V31 ? 6 : 4;
    /** clientId 长度 */
    if (!mqttClientArgs_->clientId.empty()) {
        cid_len = static_cast<unsigned short>(mqttClientArgs_->clientId.size());
    }
    else {
        cid_len = 20;
        mqttClientArgs_->clientId = "randomString"; /** FIXME: 添加随机字符串 */
        STREAM_DEBUG << "MQTT clientId :  " << mqttClientArgs_->clientId;
    }
    len += cid_len;

    if (cid_len == 0) { mqttClientArgs_->cleanSession = 1; }
    if (mqttClientArgs_->cleanSession) {
        conn_flags |= MQTT_CONN_CLEAN_SESSION;
    }

    if (mqttClientArgs_->will && !mqttClientArgs_->will->topic.empty() && !mqttClientArgs_->will->payload.empty()) {
        will_topic_len = mqttClientArgs_->will->topic_len ?
                mqttClientArgs_->will->topic_len : static_cast<unsigned short>(mqttClientArgs_->will->topic.size());
        will_payload_len = mqttClientArgs_->will->payload_len ?
                mqttClientArgs_->will->payload_len : static_cast<unsigned short>(mqttClientArgs_->will->payload.size());
        if (will_topic_len && will_payload_len) {
            conn_flags |= MQTT_CONN_HAS_WILL;
            conn_flags |= ((mqttClientArgs_->will->qos & 3) << 3);
            if (mqttClientArgs_->will->retain) {
                conn_flags |= MQTT_CONN_WILL_RETAIN;
            }
            len += 2 + will_topic_len;
            len += 2 + will_payload_len;
        }
    }
    if (!mqttClientArgs_->userName.empty()) {
        username_len = static_cast<unsigned short>(mqttClientArgs_->userName.size());
        if (username_len) {
            conn_flags |= MQTT_CONN_HAS_USERNAME;
            len += 2 + username_len;
        }
    }
    if (!mqttClientArgs_->password.empty()) {
        password_len = static_cast<unsigned short>(mqttClientArgs_->password.size());
        if (password_len) {
            conn_flags |= MQTT_CONN_HAS_PASSWORD;
            len += 2 + password_len;
        }
    }
    /** 造包 */
    MqttHead head;
    memset(&head, 0, sizeof(head));
    head.type = MQTT_TYPE_CONNECT;
    head.length = len;
    int bufLength = mqttEstimateLength(head);
    //auto buf = std::unique_ptr<unsigned char>(new unsigned char[bufLength]);
    std::unique_ptr<unsigned char[]> buf = std::make_unique<unsigned char[]>(bufLength);
    unsigned char* p = buf.get();
    int headLength = mqttHeadPack(&head, p); /** 写入MQTT头 */

    auto buffer_ = std::make_unique<Buffer>();
    buffer_->append(p,headLength);
    // TODO: Not implement MQTT_PROTOCOL_V5
    if (mqttClientArgs_->protocolVersion == MQTT_PROTOCOL_V31) {
        buffer_->appendInt8(6);
        buffer_->append(MQTT_PROTOCOL_NAME_v31, 6);
    } else {
        buffer_->appendInt8(4);
        buffer_->append(MQTT_PROTOCOL_NAME, 4);
    }
    buffer_->appendInt8(static_cast<int8_t>(mqttClientArgs_->protocolVersion));
    buffer_->appendInt8(static_cast<int8_t>(conn_flags));
    buffer_->appendInt16(static_cast<int16_t>(mqttClientArgs_->keepAlive));
    buffer_->appendInt16(static_cast<int16_t>(cid_len));

    if (cid_len > 0) {
        buffer_->append(mqttClientArgs_->clientId.c_str(), cid_len);
    }
    if (conn_flags & MQTT_CONN_HAS_WILL) {
        buffer_->appendInt16(static_cast<int16_t>(will_payload_len));
        buffer_->append(mqttClientArgs_->will->topic.c_str(), will_topic_len);
        buffer_->appendInt16(static_cast<int16_t>(will_topic_len));
        buffer_->append(mqttClientArgs_->will->payload.c_str(), will_payload_len);
    }
    if (conn_flags & MQTT_CONN_HAS_USERNAME) {
        buffer_->appendInt16(static_cast<int16_t>(username_len));
        buffer_->append(mqttClientArgs_->userName.c_str(), username_len);
    }
    if (conn_flags & MQTT_CONN_HAS_PASSWORD) {
        buffer_->appendInt16(static_cast<int16_t>(password_len));
        buffer_->append(mqttClientArgs_->password.c_str(), password_len);
    }
    /**  发送登录信息 */
    mqttHeaderCodec_.send(buffer_, headLength);
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