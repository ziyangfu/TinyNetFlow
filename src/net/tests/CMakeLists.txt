# net 单元测试
set(NET_FLOW netflow)
# boost 测试框架，后续改为gtest或者ctest
find_library(BOOSTTEST_LIBRARY NAMES boost_unit_test_framework REQUIRED)
message(STATUS "BOOSTTEST_LIBRARY = ${BOOSTTEST_LIBRARY}")

find_package(Boost REQUIRED)

find_library(BOOSTPO_LIBRARY NAMES boost_program_options)
message(STATUS "BOOSTPO_LIBRARY = ${BOOSTPO_LIBRARY}")

find_library(BOOSTSYSTEM_LIBRARY NAMES boost_system)
message(STATUS "BOOSTSYSTEM_LIBRARY = ${BOOSTSYSTEM_LIBRARY}")


if (BOOSTTEST_LIBRARY)
    add_executable(Buffer_unittest Buffer_unittest.cpp)
    target_link_libraries(Buffer_unittest ${NET_FLOW} boost_unit_test_framework)
    add_test(NAME buffer_unittest COMMAND buffer_unittest)

    add_executable(InetAddr_unittest InetAddr_unittest.cpp)
    target_link_libraries(InetAddr_unittest ${NET_FLOW} boost_unit_test_framework)
    add_test(NAME InetAddr_unittest COMMAND InetAddr_unittest)
endif ()


add_executable(Channel_test Channel_test.cpp)
target_link_libraries(Channel_test ${NET_FLOW})

add_executable(EchoClient_unittest EchoClient_unittest.cpp)
target_link_libraries(EchoClient_unittest ${NET_FLOW})

add_executable(EchoServer_unittest EchoServer_unittest.cpp)
target_link_libraries(EchoServer_unittest ${NET_FLOW})

add_executable(TcpClient_reg1 TcpClient_reg1.cpp)
target_link_libraries(TcpClient_reg1 ${NETFLOW})



add_executable(EventLoop_unittest EventLoop_unittest.cpp)
target_link_libraries(EventLoop_unittest ${NET_FLOW})

add_executable(EventLoopThread_unittest EventLoopThread_unittest.cpp)
target_link_libraries(EventLoopThread_unittest ${NET_FLOW})

add_executable(EventLoopThreadPool_unittest EventLoopThreadPool_unittest.cpp)
target_link_libraries(EventLoopThreadPool_unittest ${NET_FLOW})

add_executable(TimerQueue_unittest TimerQueue_unittest.cpp)
target_link_libraries(TimerQueue_unittest ${NET_FLOW})