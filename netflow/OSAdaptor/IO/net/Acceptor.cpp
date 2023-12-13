//
// Created by fzy on 23-5-16.
//

#include "Acceptor.h"
#include "InetAddr.h"
#include "netflow/OSAdaptor/IO/reactor/EventLoop.h"
#include "SocketsOps.h"

#include "netflow/Log/Logging.h"

#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>
#include <string>

using namespace netflow::net;

Acceptor::Acceptor(netflow::net::EventLoop *loop, const netflow::net::InetAddr &listenAddr, bool reuseport)
    :loop_(loop),
     acceptSocket_(sockets::createNonblockingSocket(listenAddr.getFamiliy())),
     acceptChannel_(loop, acceptSocket_.getFd()),
     listening_(false),
     idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) /** 满连接后的处理方法 */
{
    assert(idleFd_ >= 0);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddr(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.removeChannel();
    ::close(idleFd_);
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddr peerAddr;
    //FIXME loop until no more
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0)
    {
        std::string hostport = peerAddr.toIpPort();
        STREAM_TRACE << "Accepts of " << hostport;
        if (newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peerAddr);
        }
        else
        {
            sockets::close(connfd);
        }
    }
    else
    {
        STREAM_ERROR << "in Acceptor::handleRead";
        // Read the section named "The special problem of
        // accept()ing when you can't" in libev's doc.
        // By Marc Lehmann, author of libev.
        if (errno == EMFILE)
        {
            /** 如果服务端的连接已经满了，则使用以下方法剔除新连接 */
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.getFd(), NULL, NULL);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
