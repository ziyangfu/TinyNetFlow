set(NET_FLOW netflow)

add_executable(chat_server chat_server.cpp codec.h)
target_link_libraries(chat_server ${NET_FLOW})

add_executable(chat_client chat_client.cpp codec.h)
target_link_libraries(chat_client ${NET_FLOW})