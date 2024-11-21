file(GLOB MQTT_SRCS *.cpp)
file(GLOB MQTT_HEADER *.h)
set(NETFLOW_MQTT_SRCS ${MQTT_SRCS} CACHE INTERNAL "")


# MQTT协议单元测试
if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()