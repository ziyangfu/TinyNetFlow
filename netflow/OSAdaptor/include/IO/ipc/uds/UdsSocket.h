/** ----------------------------------------------------------------------------------------
 * \copyright
 * Copyright (c) 2023 by the TinyNetFlow project authors. All Rights Reserved.
 *
 * This file is open source software, licensed to you under the ter；ms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 * -----------------------------------------------------------------------------------------
 * \brief
 *      UNIX 域套接字socket接口
 * \details
    ipc 之 unix 域套接字的相关知识
       1. UNIX域套接字用于在同一台机器上运行的进程之间的通信。虽然因特网域套接字可用于同一目的，
          但UNIX域套接字的效率更高。
       2. UNIX域套接字仅仅复制数据；它们并不执行协议处理，不需要添加或删除网络报头，无需计算检验和，
          不要产生顺序号，无需发送确认报文。
       3. UNIX域套接字提供流和数据报两种接口。UNIX域数据报服务是可靠的，既不会丢失消息也不会传递出错。
          UNIX域套接字是套接字和管道之间的混合物。
       4. 为了创建一对非命名的、相互连接的UNIX域套接字，用户可以使用它们面向网络的域套接字接口，
          也可使用socketpair函数
 * \file
 *      UdsSocket.h
 * ----------------------------------------------------------------------------------------- */

#ifndef TINYNETFLOW_OSADAPTOR_UDSSOCKET_H
#define TINYNETFLOW_OSADAPTOR_UDSSOCKET_H

#include <sys/un.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

namespace netflow::osadaptor::ipc {

namespace udsSocket {

int createUdsSocket();
int connect(int fd, const std::string& path);
void bind(int fd, const std::string& path);
void listen(int sockfd);
int accept(int fd, const std::string& path);
ssize_t read(int fd, void* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);
int close(int sockfd);

}  // namespace udsSocket

}  // namespace netflow::osadaptor::ipc



#endif //TINYNETFLOW_OSADAPTOR_UDSSOCKET_H

