set(NET_FLOW netflow)
# UDP服务端与客户端
add_executable(chat_server_udp udpserver.cpp)
target_link_libraries(chat_server_udp ${NET_FLOW})

add_executable(chat_client_udp udpclient.cpp)
target_link_libraries(chat_client_udp ${NET_FLOW})
# UDP 组播演示
add_executable(multicast_client udpclient_multicast.cpp)
target_link_libraries(multicast_client ${NET_FLOW})

add_executable(multicast_server udpserver_multicast.cpp)
target_link_libraries(multicast_server ${NET_FLOW})