
file(GLOB NETFLOW_BASE_HEADERS *.h)
file(GLOB NETFLOW_BASE_SRCS *.cpp)
# message(STATUS "NETFLOW_BASE_HEADERS : ${NETFLOW_BASE_HEADERS}")
if (NETFLOW_BASE_HEADERS)
    message(STATUS "NETFLOW_BASE_HEADERS :  found ")
endif ()


# add_library(netflow_base STATIC ${NETFLOW_BASE_SRCS} ${NETFLOW_BASE_HEADERS})
# target_link_libraries(netflow_base pthread)

install(FILES ${NETFLOW_BASE_HEADERS} DESTINATION include/netflow/base)