//
// Created by fzy on 23-9-20.
//

#include "netflow/someip/endpoint/ClientEndpointTcp.h"

using namespace netflow::net::someip;

void ClientEndpointTcp::start() {
    connect();
}
/*!
 * \brief 发送到队列里，后续发送
 *        TODO: 调用底层网络接口，发送消息 */
void ClientEndpointTcp::sendQueued(/** 参数 */) {

}


/*! \private
 * \brief TODO: 调用底层网络库接口，建立TCP连接 */
void ClientEndpointTcp::connect() {

}

/*! \private
 * \brief TODO: 收到TCP消息， 后续还需要拆包 */
void ClientEndpointTcp::receive() {

}