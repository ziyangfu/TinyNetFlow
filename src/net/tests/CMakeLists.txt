# 单元测试
set(NET_FLOW netflow)
add_executable(Channel_test Channel_test.cpp)
target_link_libraries(Channel_test ${NET_FLOW})

add_executable(EventLoop_unittest EventLoop_unittest.cpp)
target_link_libraries(EventLoop_unittest ${NET_FLOW})