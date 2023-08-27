//
// Created by fzy on 23-8-24.
//

#ifndef TINYNETFLOW_UDPSOCKETOPS_H
#define TINYNETFLOW_UDPSOCKETOPS_H

#include <arpa/inet.h>
#include <sys/socket.h>

namespace netflow::net::udpSockets {



int createUdpSocketV4();
int createUdpSocketV6();

bool createAndBind();

/** 加入多播组 */
bool joinMulticastGroupV4(int sockfd) {
    //::setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,);
}
/** 离开多播组
 *
         * IP_MULTICAST_TTL

        设置多播组数据的TTL值

        IP_ADD_MEMBERSHIP

        在指定接口上加入组播组

        IP_DROP_MEMBERSHIP

        退出组播组

        IP_MULTICAST_IF

        获取默认接口或设置接口

        IP_MULTICAST_LOOP

        禁止组播数据回送
         */
//....

}  // namespace netflow::net::udpSockets


#endif //TINYNETFLOW_UDPSOCKETOPS_H
