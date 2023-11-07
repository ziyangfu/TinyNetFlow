file(GLOB NETFLOW_NET_SRCS ${PROJECT_SOURCE_DIR}/netflow/net/*.cpp)
# message(STATUS "NETFLOW_NET_SRCS : ${NETFLOW_NET_SRCS}")
if (NETFLOW_NET_SRCS)
    message(STATUS "NETFLOW_NET_SRCS : found")
endif ()

file(GLOB NETFLOW_BASE_SRCS ${PROJECT_SOURCE_DIR}/netflow/base/*.cpp)
if (NETFLOW_BASE_SRCS)
    message(STATUS "NETFLOW_BASE_SRCS : found")
endif ()
# message(STATUS "NETFLOW_BASE_SRCS : ${NETFLOW_BASE_SRCS}")
file(GLOB_RECURSE NETFLOW_OTHER_FILES ../base/Logging.h ../base/Glogger.h TimerId.h ../base/noncopyable.h)

# UDP 部分源文件
add_subdirectory(UDP)
# UNIX域套接字源文件
add_subdirectory(UDS)


# https://www.jianshu.com/p/e14837ae5e9a
# 方法1：
#set(NETFLOW_TOTAL_NET_SRCS ${NETFLOW_NET_SRCS} ${NETFLOW_BASE_SRCS} ${NETFLOW_OTHER_FILES}
#        PARENT_SCOPE)
# 方法2：
set(NETFLOW_TOTAL_NET_SRCS ${NETFLOW_NET_SRCS} ${NETFLOW_BASE_SRCS} ${UDP_SRCS} ${UDS_SRCS}
        CACHE INTERNAL "用于子目录传值至父目录")
# add_library(netflow_net STATIC ${NETFLOW_TOTAL_NET_SRCS})
# target_link_libraries(netflow_net pthread)


if (${BUILD_TESTS})
    add_subdirectory(./tests)
endif ()


#install(TARGETS ${NETFLOW} DESTINATION lib)

#file(GLOB_RECURSE NETFLOW_NET_HEADERS ${PROJECT_SOURCE_DIR}/netflow/net/*.h)
# message(STATUS "NETFLOW_NET_HEADERS : ${NETFLOW_NET_HEADERS}")

#install(FILES ${NETFLOW_NET_HEADERS} DESTINATION include/netflow/net)

