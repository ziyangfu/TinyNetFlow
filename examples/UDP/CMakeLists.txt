set(NET_FLOW netflow)

add_executable(chat_server_udp udpserver.cpp)
target_link_libraries(chat_server_udp ${NET_FLOW})

add_executable(chat_client_udp udpclient.cpp)
target_link_libraries(chat_client_udp ${NET_FLOW})