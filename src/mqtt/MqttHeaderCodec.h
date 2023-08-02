//
// Created by fzy on 23-7-27.
//

#ifndef TINYNETFLOW_MQTTHEADERCODEC_H
#define TINYNETFLOW_MQTTHEADERCODEC_H

#include "../../src/base/Logging.h"
#include "../../src/net/Buffer.h"
#include "../../src/net/TcpConnection.h"

#include <string>
#include <arpa/inet.h>

using namespace netflow::base;
using namespace netflow::net;
namespace netflow::net::mqtt {

class MqttHeaderCodec {
public:
    using StringMessageCallabck = std::function<void (const TcpConnectionPtr&,
                                                      const std::string& message,
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
    void send(TcpConnection* conn, std::unique_ptr<Buffer> buffer, const int len);

private:
    StringMessageCallabck messageCallback_;
};

}  // namespace netflow::net::mqtt



#endif //TINYNETFLOW_MQTTHEADERCODEC_H
