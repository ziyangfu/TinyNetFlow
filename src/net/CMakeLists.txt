file(GLOB_RECURSE NETFLOW_NET_SRCS ${PROJECT_SOURCE_DIR}/src/net/*.cpp)
# message(STATUS "NETFLOW_NET_SRCS : ${NETFLOW_NET_SRCS}")
if (NETFLOW_NET_SRCS)
    message(STATUS "NETFLOW_NET_SRCS : found")
endif ()

file(GLOB_RECURSE NETFLOW_BASE_SRCS ${PROJECT_SOURCE_DIR}/src/base/*.cpp)
if (NETFLOW_BASE_SRCS)
    message(STATUS "NETFLOW_BASE_SRCS : found")
endif ()
# message(STATUS "NETFLOW_BASE_SRCS : ${NETFLOW_BASE_SRCS}")
file(GLOB_RECURSE NETFLOW_OTHER_FILES ../base/Logging.h ../base/Glogger.h TimerId.h ../base/noncopyable.h)

file(GLOB_RECURSE NETFLOW_TOTAL_SRCS ${NETFLOW_NET_SRCS} ${NETFLOW_BASE_SRCS} ${NETFLOW_OTHER_FILES})

# compile static lib
add_library(${NETFLOW} STATIC ${NETFLOW_TOTAL_SRCS})
target_link_libraries(${NETFLOW} pthread)

if (${BUILD_TESTS})
    add_subdirectory(./tests)
endif ()


install(TARGETS ${NETFLOW} DESTINATION lib)

file(GLOB_RECURSE NETFLOW_NET_HEADERS ${PROJECT_SOURCE_DIR}/src/net/*.h)
# message(STATUS "NETFLOW_NET_HEADERS : ${NETFLOW_NET_HEADERS}")

install(FILES ${NETFLOW_NET_HEADERS} DESTINATION include/netflow/net)

