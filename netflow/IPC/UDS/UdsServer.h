//
// Created by fzy on 23-11-16.
//

#ifndef TINYNETFLOW_UDSSERVER_H
#define TINYNETFLOW_UDSSERVER_H

#include "netflow/IPC/UDS/PreDefine.h"

namespace netflow::net {
/*!
 * \brief 使用 Unix 域套接字这种IPC方式的服务端, receiver */
class UdsServer {
public:
    UdsServer();
    ~UdsServer();

    void bind();
    void listen();
    void accept();

    void start();
    void stop();

    void onMessageCallback();

    void setThreadNums(int threadNum);


private:

};

}
#endif //TINYNETFLOW_UDSSERVER_H
