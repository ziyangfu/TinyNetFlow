//
// Created by fzy on 2025/1/13.
//
#include "IO/ipc/uds/Acceptor.h"
#include "IO/ipc/uds/UdsSocket.h"


#if 0

namespace osadaptor {
namespace ipc {




Acceptor::Acceptor() {
    acceptSocket_ = udsSocket::createUdsSocket();
}


Acceptor::~Acceptor() {

}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();          /** 开启监听模式 */
    acceptChannel_.enableReading();  /** 注册进epoll */
}


void Acceptor::setNewConnectionCallback(const osadaptor::ipc::Acceptor::NewConnectionCallback &cb) {
    newConnectionCallback_ = std::move(cb);
}


/*!
 * \brief 1. uds连接； 2. shared memory “连接” 建立处理函数，负责处理传递过来的memfd
 * */
void Acceptor::handleReactorEventRead() {
    int connFd_ = ::accept();
    int memfd = udsSocket::recvMsgWithMemFd(udsAcceptFd_);
    /** msg的主消息 */
    if (/** dummy == shm protocol */ 1) {
        isShmConnection_ = true;
    }

}



}  // namespace ipc
} // namespace osadaptor

#endif