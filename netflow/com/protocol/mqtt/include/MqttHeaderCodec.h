#ifndef COM_PROTOCOL_MQTT_MQTT_HEADER_CODEC_H
#define COM_PROTOCOL_MQTT_MQTT_HEADER_CODEC_H

#include "IO/net/Buffer.h"
#include "IO/net/TcpConnection.h"

#include <arpa/inet.h>
#include <string>

namespace com {

class MqttHeaderCodec {
public:
    using StringMessageCallabck = std::function<void (const osadaptor::net::TcpConnectionPtr&,
                                                      osadaptor::net::Buffer& buf,
                                                      osadaptor::time::Timestamp)>;
    explicit MqttHeaderCodec(const StringMessageCallabck& cb)
            : messageCallback_(cb)
    {}
    /*!
     * \brief TCP按照协议头长度拆包 */
    void onMessage(const osadaptor::net::TcpConnectionPtr& conn,
                   osadaptor::net::Buffer* buf, osadaptor::time::Timestamp receiveTime);
/*!
* \brief: 发送消息 */
    void send(osadaptor::net::TcpConnection* conn, const std::string& message);
    void send(osadaptor::net::TcpConnection* conn, const char* message, const int len);
    void send(osadaptor::net::TcpConnection* conn, std::unique_ptr<osadaptor::net::Buffer> buffer);

private:
    StringMessageCallabck messageCallback_;
};

}  // namespace com



#endif //COM_PROTOCOL_MQTT_MQTT_HEADER_CODEC_H
