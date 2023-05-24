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
/*!
 * \brief 全局函数，设置初始连接回调，什么事也不做 */
void netflow::net::defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    //LOG_TRACE << conn->localAddress().toIpPort() << " -> "
     //         << conn->peerAddress().toIpPort() << " is "
     //         << (conn->connected() ? "UP" : "DOWN");
    // do not call conn->forceClose(), because some users want to register message callback only.
}
/*!
 * \brief 全局函数，设置初始消息回调， buffer的读写“指针”设置在初始点 */
void netflow::net::defaultMessageCallback(const TcpConnectionPtr&, Buffer* buf)  // TODO  Timestamp
{
    buf->retrieveAll();
}

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
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::forceClose() {
    if (state_ == kConnected || state_ == kDisconnecting) {
        setState(kDisconnecting);
        loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseWithDelay(double seconds) {

}

void TcpConnection::setTcpNoDelay(bool on) {
    socket_->setTcpNoDelay(on);
}

void TcpConnection::startRead() {
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::stopRead() {
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    // channel_.tie(shared_from_this());  // TODO
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->removeChannel();
}

/*!
 * \private 私有成员函数
 * **********************************************************************************/
void TcpConnection::handleRead() {
    loop_->assertInLoopThread();
    int saveError = 0;
    /** 从 socket缓冲区读取数据到 inputBuffer */
    ssize_t n = inputBuffer_.readFd(channel_->getFd(), &saveError);
    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_);
    }
    /** 没读到数据 */
    else if (n == 0) {
        handleClose();
    }
    else {
        errno = saveError;
        handleError();
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    if (channel_->isWriting()) {
        ssize_t n = sockets::write(channel_->getFd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        }
        /** n <= 0 */
        else {
            /** log system error */
        }
    }
    else {
        /** socket 连接关闭，没有数据写 */
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    channel_->disableAll();
    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);  /* TODO */
    /** 必须在最后一行 */
    closeCallback_(guardThis);
}

void TcpConnection::handleError() {
    int err = sockets::getSocketError(channel_->getFd());
    /** log error: err */
}

void TcpConnection::sendInLoop(const void *message, size_t len) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (state_ == kDisconnected) {
        return;
    }
    /** 若buffer中没有数据，尝试直接发 */
    if ( !channel_->isWriting() && (outputBuffer_.readableBytes() == 0) ) {
        nwrote = sockets::write(channel_->getFd(), message, len);
        /** 返回共发出去多少字节，或者返回-1（出错）*/
        if (nwrote >= 0) {
            remaining = len - nwrote;
            /** 全发出去了 */
            if (remaining == 0 && writeCompleteCallback_) {
                /* FIXME: 为什么要用 shared_from_this() */
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                /** log system error: TcpConnection::sendInLoop */
                if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                {
                    faultError = true;
                }
            }
        }
    }
    /** 追加到outputBuffer_
     * 1. outputBuffer_ 中本来就有数据，因此将新发送数据追加到buffer后面
     * 2. outputBuffer_ 中原本没有数据，本次发送时，一次没有完整发出去，将本次剩下的数据追加到buffer后面 */
    assert(remaining <= len);
    if (!faultError && remaining > 0) {
        ssize_t oldlen = outputBuffer_.readableBytes();
        /** outputBuffer 容量不够，执行高水位回调进行扩容 */
        if (oldlen + remaining >= highWaterMark_
            && oldlen < highWaterMark_
            && highWaterMarkCallback_) {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldlen + remaining));
        }
        /** 数据指针要加上 nwrote的原因：
         * 1. message全部追加到buffer中，此时 nwrote == 0
         * 2. message已经发出去了一部分，共计 nwrote， 那么数据起始指针需要向右移动 nwrote */
        outputBuffer_.append(static_cast<const char*>(message) + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();  /** 触发写事件 */
        }
    }
}

void TcpConnection::sendInLoop(const std::string &message) {
    sendInLoop(message.c_str(), message.size());
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

void TcpConnection::forceCloseInLoop() {
    loop_->assertInLoopThread();
    if (state_ == kConnected || state_ == kDisconnecting) {
        handleClose();
    }
}

const char *TcpConnection::stateToString() const {
    switch (state_)
    {
        case kDisconnected:
            return "kDisconnected";
        case kConnecting:
            return "kConnecting";
        case kConnected:
            return "kConnected";
        case kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
    }
}

void TcpConnection::startReadInLoop() {
    loop_->assertInLoopThread();
    if (!reading || !channel_->isReading()) {
        channel_->enableReading();
        reading = true;
    }
}

void TcpConnection::stopReadInLoop() {
    loop_->assertInLoopThread();
    if (reading || channel_->isReading()) {
        channel_->disableReading();
        reading = false;
    }

}