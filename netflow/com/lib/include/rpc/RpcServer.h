


#ifndef COM_RPC_RPC_SERVER_H
#define COM_RPC_RPC_SERVER_H

#include "IO/net/TcpServer.h"
#include "rpc/RpcService.h"

/*!
 * 服务与method提供方，注册服务
 * */


namespace com::rpc
{
class Service;

class RpcServer
{
public:
  RpcServer(osadaptor::net::EventLoop* loop,
            const osadaptor::net::InetAddr& listenAddr);

  void setThreadNum(int numThreads)
  {
    server_.setThreadNum(numThreads);
  }

  // NOT thread safe, must call before start().
  void registerService(Service*);
  void start();

 private:
  void onConnection(const osadaptor::net::TcpConnectionPtr& conn);

  // void onMessage(const TcpConnectionPtr& conn,
  //                Buffer* buf,
  //                Timestamp time);
private:

    osadaptor::net::EventLoop* loop_;
    osadaptor::net::TcpServer server_;
    std::map<std::string, Service*> services_;
    RpcServiceImpl metaService_;
};

}  // namespace com::rpc

#endif  // COM_RPC_RPC_SERVER_H


// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.