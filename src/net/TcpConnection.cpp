//
// Created by fzy on 23-5-17.
//

#include "TcpConnection.h"

#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketsOps.h"

#include <errno.h>

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
    //
}