//
// Created by fzy on 23-5-17.
//

#include "TcpConnection.h"

#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOps.h"

#include <errno.h>
#include <string_view>  /** replace StringPiece */

using namespace netflow::net;

TcpConnection::TcpConnection(netflow::net::EventLoop *loop, const std::string &name, int sockfd,
                             const netflow::net::InetAddr &localAddr, const netflow::net::InetAddr &peerAddr)
        : loop_(loop),
          name_(name),
          socket_(std::make_unique<Socket>(sockfd)),
          channel_(std::make_unique<Channel>(loop_, sockfd)),
          localAddr_(localAddr),
          peerAddr_(peerAddr),
          state_(kConnecting),
          reading(true),
          highWaterMark_(64*1024*1024)
{
    /** 设置建立连接时的回调
     * bind绑定类成员函数时，第一个参数表示对象的成员函数的指针，第二个参数表示对象的地址
     * std::bind(callableFunc,_1,2)等价于std::bind (&callableFunc,_1,2) */
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    /** 设置 TCP 保活机制 */
    socket_->setKeepAlive(true);
}
TcpConnection::~TcpConnection() {
    assert(state_ == kDisconnected);
}

void TcpConnection::send(const void *message, int len) {
    //
}

void TcpConnection::send(const std::string &message) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThead()) {
            sendInLoop(message);
        }
        else {
            /** 获得 sendInLoop的函数指针 */
            void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
            /** 唤醒后在IO线程中执行 */
            loop_->runInLoop(std::bind(fp, this, message))  /** TODO: string_view */
        }
    }

}

void TcpConnection::send(netflow::net::Buffer *buf) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThead()) {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();  /** buffer 中数据全部发出，读写指针退回初始状态 */
        }
        else {
            void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, buf->retrieveAllAsString()))
        }
    }

}

void TcpConnection::shutdown() {

}

void TcpConnection::forceClose() {

}

void TcpConnection::forceCloseWithDelay(double seconds) {

}

void TcpConnection::setTcpNoDelay(bool on) {

}

void TcpConnection::startRead() {

}

void TcpConnection::stopRead() {

}

void TcpConnection::connectEstablished() {

}

void TcpConnection::connectDestroyed() {

}

/*!
 * \private 私有成员函数
 * **********************************************************************************/
void TcpConnection::handleRead() {

}

void TcpConnection::handleWrite() {

}

void TcpConnection::handleClose() {

}

void TcpConnection::handleError() {

}

void TcpConnection::sendInLoop(const void *message, size_t len) {

}

void TcpConnection::sendInLoop(const std::string &message) {
    sendInLoop(message.c_str(), message.size());
}

void TcpConnection::shutdownInLoop() {

}

void TcpConnection::forceCloseInLoop() {

}

const char *TcpConnection::stateToString() const {

}

void TcpConnection::startReadInLoop() {

}

void TcpConnection::stopReadInLoop() {

}