set(NET_FLOW netflow)
add_executable(mqttClientPub mqttClientPub.cpp)
target_link_libraries(mqttClientPub ${NET_FLOW})

add_executable(mqttClientSub mqttClientSub.cpp)
target_link_libraries(mqttClientSub ${NET_FLOW})