file(GLOB HTTP_SRCS *.cpp)
file(GLOB HTTP_HEADER *.h)

set(NETFLOW_HTTP_SRCS ${HTTP_SRCS} CACHE INTERNAL "用于子目录传值至父目录")
# add_library(netflow_http STATIC ${NETFLOW_HTTP_SRCS})
# target_link_libraries(netflow_http ${NETFLOW})

if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()