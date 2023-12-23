//
// Created by fzy on 23-12-21.
//

#ifndef TINYNETFLOW_IPCMEDIAADDR_H
#define TINYNETFLOW_IPCMEDIAADDR_H

/** 定义 UDS 与 shared memory 的统一地址 ， 采用 domain与port来描述 */

struct UnixDomainPath {
    int domain;
    int port;
};

#endif //TINYNETFLOW_IPCMEDIAADDR_H
