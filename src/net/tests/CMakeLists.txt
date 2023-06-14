# net 单元测试
set(NET_FLOW netflow)
if (BOOSTTEST_LIBRARY)
    add_executable(Buffer_unittest Buffer_unittest.cpp)
    target_link_libraries(Buffer_unittest ${NET_FLOW})

    add_executable(InetAddr_unittest InetAddr_unittest.cpp)
    target_link_libraries(InetAddr_unittest ${NET_FLOW})
endif ()


add_executable(Channel_test Channel_test.cpp)
target_link_libraries(Channel_test ${NET_FLOW})

add_executable(EchoClient_unittest EchoClient_unittest.cpp)
target_link_libraries(EchoClient_unittest ${NET_FLOW})

add_executable(EchoServer_unittest EchoServer_unittest.cpp)
target_link_libraries(EchoServer_unittest ${NET_FLOW})

add_executable(EventLoop_unittest EventLoop_unittest.cpp)
target_link_libraries(EventLoop_unittest ${NET_FLOW})

add_executable(EventLoopThread_unittest EventLoopThread_unittest.cpp)
target_link_libraries(EventLoopThread_unittest ${NET_FLOW})

add_executable(EventLoopThreadPool_unittest EventLoopThreadPool_unittest.cpp)
target_link_libraries(EventLoopThreadPool_unittest ${NET_FLOW})

add_executable(TimerQueue_unittest TimerQueue_unittest.cpp)
target_link_libraries(TimerQueue_unittest ${NET_FLOW})