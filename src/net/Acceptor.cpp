//
// Created by fzy on 23-5-16.
//

#include "Acceptor.h"
#include "InetAddr.h"
#include "EventLoop.h"
#include "SocketsOps.h"

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

using namespace netflow::net;

Acceptor::Acceptor(netflow::net::EventLoop *loop, const netflow::net::InetAddr &listenAddr, bool reuseport)
    :loop_(loop),
     acceptSocket_(sockets::createNonblockingSocket(listenAddr.getFamiliy())),
     acceptChannel_(loop, acceptSocket_.getFd()),
     listening_(false),
     idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC);) /** 满连接后的处理方法 */
{
    assert(idleFd_ >= 0);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bind(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);
}

void Acceptor::listen() {
    //loop_->assertInLoopThread();
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
    //loop_->assertInLoopThread();
    InetAddr peerAddr;
    //FIXME loop until no more
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0)
    {
        // string hostport = peerAddr.toIpPort();
        // LOG_TRACE << "Accepts of " << hostport;
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
        //LOG_SYSERR << "in Acceptor::handleRead";
        // Read the section named "The special problem of
        // accept()ing when you can't" in libev's doc.
        // By Marc Lehmann, author of libev.
        if (errno == EMFILE)
        {
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
