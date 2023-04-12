set(NET_CURRENT_DIR ${PROJECT_SOURCE_DIR}/src/net)
file(GLOB_RECURSE NET_SRCS ${NET_CURRENT_DIR}/*.cpp ${NET_CURRENT_DIR}/poller/*.cpp)

add_library(muduo_net ${NET_SRCS} connector.cpp)
target_link_libraries(muduo_net muduo_base)