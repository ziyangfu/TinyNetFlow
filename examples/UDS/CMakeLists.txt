set(NET_FLOW netflow)
# UDP服务端与客户端
add_executable(uds_server uds_server.cpp)
target_link_libraries(uds_server ${NET_FLOW})

add_executable(uds_client uds_client.cpp)
target_link_libraries(uds_client ${NET_FLOW})