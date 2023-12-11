//
// Created by fzy on 23-7-27.
//

#ifndef TINYNETFLOW_MQTTHEADERCODEC_H
#define TINYNETFLOW_MQTTHEADERCODEC_H

#include "netflow/base/Logging.h"
#include "netflow/net/Buffer.h"
#include "netflow/net/TcpConnection.h"

#include <arpa/inet.h>
#include <string>

using namespace netflow::base;
using namespace netflow::net;
namespace netflow::net::mqtt {

class MqttHeaderCodec {
public:
    using StringMessageCallabck = std::function<void (const TcpConnectionPtr&,
                                                      Buffer& buf,
                                                      Timestamp)>;
    explicit MqttHeaderCodec(const StringMessageCallabck& cb)
            : messageCallback_(cb)
    {}
    /*!
     * \brief TCP按照协议头长度拆包 */
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime);
/*!
* \brief: 发送消息 */
    void send(TcpConnection* conn, const std::string& message);
    void send(TcpConnection* conn, const char* message, const int len);
    void send(TcpConnection* conn, std::unique_ptr<Buffer> buffer);

private:
    StringMessageCallabck messageCallback_;
};

}  // namespace netflow::net::mqtt



#endif //TINYNETFLOW_MQTTHEADERCODEC_H
