file(GLOB MQTT_SRCS ${PROJECT_SOURCE_DIR}/src/mqtt/*.cpp)
file(GLOB MQTT_HEADER ${PROJECT_SOURCE_DIR}/src/mqtt/*.h)
set(NETFLOW_MQTT_SRCS ${MQTT_SRCS} CACHE INTERNAL "")


# MQTT协议单元测试
if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()