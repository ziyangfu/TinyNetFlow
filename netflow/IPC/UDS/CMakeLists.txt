message(STATUS "enable Unix domain socket protocol!")
file(GLOB NET_UDS_SRCS ${PROJECT_SOURCE_DIR}/netflow/net/UDS/*cpp)
file(GLOB NET_UDS_HEADERS ${PROJECT_SOURCE_DIR}/netflow/net/UDS/*.h)
set(UDS_SRCS ${NET_UDS_SRCS} CACHE INTERNAL "Unix Domain Socket")