file(GLOB_RECURSE MQTT_SRCS ${PROJECT_SOURCE_DIR}/src/mqtt/*.cpp)
file(GLOB_RECURSE MQTT_HEADER ${PROJECT_SOURCE_DIR}/src/mqtt/*.h)
file(GLOB_RECURSE MQTT_TOTAL_SRCS ${MQTT_SRCS} ${MQTT_HEADER})
# 后续所有 net， http， mqtt 合成一个静态链接库
add_library(netflow_mqtt STATIC ${MQTT_TOTAL_SRCS})
target_link_libraries(netflow_mqtt ${NETFLOW})

# MQTT协议单元测试
if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()