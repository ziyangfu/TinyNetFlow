cmake_minimum_required(VERSION 3.16)
project(TinyNetFlow)

set(CMAKE_CXX_STANDARD 17)
# -----------------------------------------------------------------------------
option(BUILD_EXAMPLE "build examples" ON)
option(BUILD_TESTS "build unittest" OFF)

option(BUILD_HTTP "build http protocol" OFF)
option(BUILD_MQTT "build mqtt protocol" OFF)
option(BUILD_SOMEIP "build someip protocol" OFF)

option(USE_GLOG "using glog" OFF)
# -----------------------------------------------------------------------------
set(NETFLOW netflow)


if (USE_GLOG)
    find_package(glog REQUIRED)
    message(STATUS "find package: glog")
else ()
    message(STATUS "Logger will using spdlog")
endif ()

# 0 ~ 5 TRACE DEBUG INFO WARN ERROR FATAL
# log level
# set(LOG_LEVEL 0)
# message(STATUS "now log level is ${LOG_LEVEL}")


# set compile flags
set(${CMAKE_CXX_FLAGS}
        -g
        # -DVALGRIND
        -DCHECK_PTHREAD_RETURN_VALUE
        -D_FILE_OFFSET_BITS=64
        -Wall
        -Wextra
        -Werror
        -Wconversion
        -Wno-unused-parameter
        -Wold-style-cast
        -Woverloaded-virtual
        -Wpointer-arith
        -Wshadow
        -Wwrite-strings
        -march=native
        # -MMD
        -std=c++17
        -rdynamic
)
#string(REPLACE ";" " " ${CMAKE_CXX_FLAGS} "${CXX_FLAGS})
set(${CMAKE_CXX_FLAGS_DEBUG} "-O0")
set(${CMAKE_CXX_FLAGS_RELEASE} "-O2")   # 编译器 O2 优化
# 默认安装路径
set(INSTALL_DIR ${PROJECT_SOURCE_DIR}/build/install)
include_directories(${PROJECT_SOURCE_DIR})
# third_party
include_directories("${PROJECT_SOURCE_DIR}/third_src/spdlog/include/")
# 底层网络库部分
add_subdirectory(netflow/OSLayer/base)
add_subdirectory(netflow/OSLayer/net)
# IPC部分，包括unix domain socket、shared memory、signal
add_subdirectory(netflow/OSLayer/IPC/UDS)
add_subdirectory(netflow/OSLayer/IPC/shm)
add_subdirectory(netflow/OSLayer/IPC/signal)
set(NETFLOW_IPC ${UDS_SRCS} ${IPC_SHM_SRCS} ${IPC_SIGNAL_SRCS})

set(NETFLOW_TOTAL_SRCS ${NETFLOW_TOTAL_NET_SRCS}  ${NETFLOW_IPC})

if (BUILD_HTTP)
    message(STATUS "build HTTP protocol : ON")
    add_subdirectory(netflow/Com/http)
    list(APPEND NETFLOW_TOTAL_SRCS "${NETFLOW_HTTP_SRCS}")
    message(STATUS "Added HTTP protocol source file")
else ()
    message(STATUS "build HTTP protocol : OFF")
endif ()

if (BUILD_MQTT)
    message(STATUS "build MQTT protocol : ON")
    add_subdirectory(netflow/Com/mqtt)
    list(APPEND NETFLOW_TOTAL_SRCS "${NETFLOW_MQTT_SRCS}")
    message(STATUS "Added MQTT protocol source file")
else ()
    message(STATUS "build MQTT protocol : OFF")
endif ()

if (BUILD_SOMEIP)
    message(STATUS "build SOME/IP protocol : ON")
    add_subdirectory(netflow/Com/someip)
    list(APPEND NETFLOW_TOTAL_SRCS "${NETFLOW_SOMEIP_SRCS}")
    message(STATUS "Added SOMEIP protocol source file")
else ()
    message(STATUS "build SOME/IP protocol : OFF")
endif ()
# compile static lib
add_library(${NETFLOW} STATIC ${NETFLOW_TOTAL_SRCS})
target_link_libraries(${NETFLOW} pthread)

if (BUILD_EXAMPLE)
    message(STATUS "build examples : ON")
    add_subdirectory(examples)
else ()
    message(STATUS "build examples : OFF")
endif ()