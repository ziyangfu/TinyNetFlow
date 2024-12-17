# add_subdirectory(chat)

if (BUILD_MQTT)
    add_subdirectory(com/mqtt)
endif ()
# add_subdirectory(UDP)

add_subdirectory(osadaptor/ipc/UDS)