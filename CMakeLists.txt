cmake_minimum_required(VERSION 3.16)
project(TinyNetFlow)

set(CMAKE_CXX_STANDARD 17)
# -----------------------------------------------------------------------------
option(BUILD_EXAMPLE "build examples" OFF)
option(BUILD_TESTS "build unittest" ON)

option(BUILD_HTTP "build http protocol" OFF)
option(BUILD_MQTT "build mqtt protocol" OFF)
option(BUILD_SOMEIP "build someip protocol" OFF)

option(USE_GLOG "using glog" OFF)
# -----------------------------------------------------------------------------

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
# 默认安装路径
set(INSTALL_DIR ${PROJECT_SOURCE_DIR}/build/install)

# third_party
include_directories("${PROJECT_SOURCE_DIR}/third_src/spdlog/include/")

add_subdirectory(${PROJECT_SOURCE_DIR}/src/base)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/net)

if (BUILD_EXAMPLE)
    message(STATUS "build examples : ON")
    add_subdirectory(examples)
else ()
    message(STATUS "build examples : OFF")
endif ()


if (BUILD_HTTP)
    message(STATUS "build HTTP protocol : ON")
    add_subdirectory(src/http)
else ()
    message(STATUS "build HTTP protocol : OFF")
endif ()

if (BUILD_MQTT)
    message(STATUS "build MQTT protocol : ON")
    add_subdirectory(src/mqtt)
else ()
    message(STATUS "build MQTT protocol : OFF")
endif ()