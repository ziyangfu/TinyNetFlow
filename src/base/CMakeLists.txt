
file(GLOB NETFLOW_BASE_HEADERS ${PROJECT_SOURCE_DIR}/src/base/*.h)
file(GLOB NETFLOW_BASE_SRCS ${PROJECT_SOURCE_DIR}/src/base/*.cpp)
# message(STATUS "NETFLOW_BASE_HEADERS : ${NETFLOW_BASE_HEADERS}")
if (NETFLOW_BASE_HEADERS)
    message(STATUS "NETFLOW_BASE_HEADERS :  found ")
endif ()


# add_library(netflow_base STATIC ${NETFLOW_BASE_SRCS} ${NETFLOW_BASE_HEADERS})
# target_link_libraries(netflow_base pthread)

install(FILES ${NETFLOW_BASE_HEADERS} DESTINATION include/netflow/base)