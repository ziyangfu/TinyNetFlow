message(STATUS "enable Unix domain socket protocol!")
file(GLOB NET_UDS_SRCS *cpp)
file(GLOB NET_UDS_HEADERS *.h)
set(UDS_SRCS ${NET_UDS_SRCS} CACHE INTERNAL "Unix Domain Socket")