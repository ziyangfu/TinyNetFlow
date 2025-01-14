//
// Created by fzy on 23-11-16.
//

#include "IO/ipc/uds/UdsServer.h"

#include "IO/reactor/EventLoop.h"
#include "IO/reactor/Channel.h"
#include "IO/reactor/EventLoopThreadPool.h"
#include "IO/net/InetAddr.h"   /** 为了适配 acceptor 而引入，实际unix domain socket 不需要 */

#include <spdlog/spdlog.h>


#include <sys/mman.h>  /** 临时使用 */

using namespace osadaptor::ipc;
using namespace osadaptor::net;


/** static */ const int UdsServer::kBufferSize = 1400;

UdsServer::UdsServer(EventLoop* loop, const std::string& name,
                     struct uds::UnixDomainPath path /** == uds::UnixDomainDefaultPath */)
    : sockfd_(udsSocket::createUdsSocket()),
      path_(path),
      unixDomainStringPath_(generateUnixDomainPath()),
      loop_(loop),
      name_(name),
      isConnected_(false),
      connectedChannel_(nullptr),
      threadPool_(std::make_shared<EventLoopThreadPool>(loop_, name_))
{
    //acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
    //                                              std::placeholders::_1, std::placeholders::_2));



    loop_->runInLoop([this](){
        bind();
        listen();
        //accept();
        channel_ = std::make_unique<Channel>(loop_, sockfd_);
        channel_->setReadCallback(std::bind(&UdsServer::onAccept, this));
        channel_->enableReading();
    });
}


UdsServer::~UdsServer() {
    close();
}

/*!
 * \brief 服务端启动被动监听模式
 * */
void UdsServer::start() {
#if 0
    if (!started_) {
        threadPool_->start(threadInitCallback_);    /** 开始创建线程池 */
        /** 此时还没有开始listen模式 */
        assert(!acceptor_->listening());
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
#endif
}

void UdsServer::stop() {

}

void UdsServer::close() {
    loop_->runInLoop([this]() {
        channel_->disableAll();
        udsSocket::close(sockfd_);
        ::unlink(unixDomainStringPath_.c_str());  /** 删除路径 */
    });

}

void UdsServer::bind() {
    udsSocket::bind(sockfd_, unixDomainStringPath_);
}

void UdsServer::listen() {
    udsSocket::listen(sockfd_);
}

int UdsServer::accept() {
    return udsSocket::accept(sockfd_, unixDomainStringPath_);
}

void UdsServer::send(const std::string &message) {
    if (loop_->isInLoopThread()) {
        sendInLoop(message);
    }
    else {
        void (UdsServer::*fp)(const std::string& message) = &UdsServer::sendInLoop;
        loop_->runInLoop(std::bind(fp, this, message));
    }
}

/*!
 * \brief 接收memfd
 * */
int UdsServer::recvMemFd() {
    //return udsSocket::recvMsgWithMemFd(sockfd_);
}



void UdsServer::setThreadNums(int threadNum) {
    assert(threadNum >= 0);
    threadPool_->setThreadNum(threadNum);
}

void UdsServer::setMessageCallback(UdsServer::MessageCb cb) {
    messageCallback_ = std::move(cb);
}

void UdsServer::setConnectionCallback(UdsServer::ConnectionCb cb) {
    connectionCallback_ = std::move(cb);
}

/*!
 * \brief shm “连接”建立好的回调函数
 * */
void UdsServer::setShmConnectionCallback(osadaptor::ipc::UdsServer::ConnectionCb cb) {
    shmConnectedCallback_ = std::move(cb);
}

void UdsServer::newConnection(int sockfd, const InetAddr &peerAddr) {

}

void UdsServer::removeConnection() {

}

void UdsServer::removeConnectionInLoop() {

}

std::string UdsServer::generateUnixDomainPath() {
    std::string str;
    if (path_.domain == 10 && path_.port == 10) {
        str = uds::kUnixDomainDefaultPathString;
    }
    else {
        str = uds::kUnixDomainPathFirstString + std::to_string(path_.domain)
              + uds::kUnixDomainPathSecondString + std::to_string(path_.port);
    }
    SPDLOG_TRACE("unix domain socket path is ");
    return str;
}

/*!
 * \brief accept回调函数
 * */
void UdsServer::onAccept() {
    loop_->assertInLoopThread();
    /** 去除原来的channel */
    clientFd_ = accept();
    isConnected_ = true;
    SPDLOG_INFO("new unix domain socket connection, client fd is {}", clientFd_);
    connectedChannel_ = std::make_unique<Channel>(loop_, clientFd_);
    connectedChannel_->setReadCallback(std::bind(&UdsServer::handleRead, this, std::placeholders::_1));
    connectedChannel_->setErrorCallback(std::bind(&UdsServer::handleError, this));
    connectedChannel_->setCloseCallback(std::bind(&UdsServer::handleClose, this));
    connectedChannel_->enableReading();
}

/*!
 * \brief 在loop所在线程发送数据
 * \private
 * */
void UdsServer::sendInLoop(const std::string &message) {
    loop_->assertInLoopThread();
    sendInLoop(message.c_str(), message.length());
}

void UdsServer::sendInLoop(const void *message, size_t len) {
    /**  若是 “ 已连接 ” 状态， 即提前保存了目标地址 */
    if (isConnected_) {
        udsSocket::write(clientFd_, message, len);
    }
    else {
        /** 没有提前设置地址，报错 */
        SPDLOG_ERROR("uds ： must to call connect() to set remote address before now");
    }
}

/*! \brief 读事件处理函数
 * \details 如果是普通消息，执行上层的消息回调
 *          如果是shm connection消息，则执行shm connection回调
 *          为了兼容memfd的描述符传递，统一使用 recvmsg()
 * */
void UdsServer::handleRead(time::Timestamp receiveTime) {
    loop_->assertInLoopThread();
    std::string message{};
    std::pair<std::size_t, std::optional<int>> recv;
    recv = udsSocket::recvMsg(clientFd_, message);
    /** recvBytes */
    if (recv.first > 0) {
        messageCallback_(message, receiveTime);
    }
    /** 没读到数据 */
    else if (recv.first == 0) {
        handleClose();
    }
    else {
        //errno = saveError;
        SPDLOG_TRACE("unix domain socket error, bytesRead is {}", bytesRead);
        handleError();
    }

    /** exist memfd
     * 或者交给shm部分来处理？ 这样貌似更好
     * */
    if (recv.second.has_value()) {
        const size_t size {4096}; /** fixme */
        void* addrPtr {nullptr};
        addrPtr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, memFd_.value(), 0);
        auto ptr = reinterpret_cast<std::uint8_t*>(addrPtr);
        /** ptr 映射ringbuffer */
        if (shmConnectedCallback_) {
            shmConnectedCallback_(receiveTime);
        }
    }
}

void UdsServer::handleClose() {
    SPDLOG_TRACE("close unix domain socket now");
}

void UdsServer::handleError() {
    SPDLOG_TRACE("unix domain socket meeting an error, will abort now");
    abort();
}