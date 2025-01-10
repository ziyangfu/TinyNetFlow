# \copyright
# Copyright (c) 2023 by the TinyNetFlow project authors. All Rights Reserved.
#
# This file is open source software, licensed to you under the terms
# of the Apache License, Version 2.0 (the "License").  See the NOTICE file
# distributed with this work for additional information regarding copyright
# ownership.  You may not use this file except in compliance with the License.
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
# -----------------------------------------------------------------------------------------
# \brief
#       TinyNetFlow 顶层CMakeLists文件
# \details
#       支持（规划）的静态链接库：
#           osadaptor： 操作系统适配层，封装OS底层系统调用，同时为未来支持多个OS平台做准备
#           com      ： 通信库，主要支持HTTP、MQTT、SOME/IP、SOME/IP-SD等协议，提供多种协议绑定
#                       提供someipd守护进程
#           em       ： 执行管理模块，负责程序的全生命周期管理，启动、停止、重启等
#                       提供execmd守护进程
#       支持的OS：
#           Linux
# -----------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.16)
project(TinyNetFlow
        LANGUAGES CXX
        VERSION 0.0.1
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "TinyNetFlow build with C++ standard: ${CMAKE_CXX_STANDARD}")

# OS部分，目前仅支持Linux
if (NOT DEFINED OS_PLATFORM)
    string(TOLOWER ${CMAKE_SYSTEM_NAME} OS_PLATFORM)
endif ()
if (OS_PLATFORM MATCHES "linux")
    include(GNUInstallDirs)  # for CMAKE_INSTALL_INCLUDEDIR ......
    INCLUDE(CheckFunctionExists) # for command check_function_exists ......
    message(STATUS "TinyNetFlow build in OS platform: Linux")
else ()
    message(SEND_ERROR "TinyNetFlow only support Linux platform now")
endif ()

if (DEFINED CMAKE_BUILD_TYPE)
    message(STATUS "TinyNetFlow lib build type: ${CMAKE_BUILD_TYPE}")
endif ()
# 设置Debug模式的编译选项
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g
#                 -O0
#                 -DCHECK_PTHREAD_RETURN_VALUE
#                 -D_FILE_OFFSET_BITS=64
#                 -Wall
#                 -Wextra
#                 -Werror
#                 -Wconversion
#                 -Wno-unused-parameter
#                 -Wold-style-cast
#                 -Woverloaded-virtual
#                 -Wpointer-arith
#                 -Wshadow
#                 -Wwrite-strings
#                 -march=native
#                 -rdynamic")
set(CXX_FLAGS
        -g
        # -DVALGRIND
        -DCHECK_PTHREAD_RETURN_VALUE
        -D_FILE_OFFSET_BITS=64
        -Wall
        -Wextra
        #-Werror
        -Wconversion
        -Wno-unused-parameter
        -Wold-style-cast
        -Woverloaded-virtual
        -Wpointer-arith
        -Wshadow
        -Wwrite-strings
        -march=native
        # -MMD
        -rdynamic
)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

# 设置Release模式的编译选项
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")


# 设置默认安装目录 <build_dir>/install
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)
message(STATUS "project install dir : ${CMAKE_INSTALL_PREFIX}")
# ---------------------------------------------------------------------
option(BUILD_OSADAPTOR  "is build osadaptor lib"        ON  )
option(BUILD_COM        "is build com lib and daemon"   OFF )
option(BUILD_EM         "is build em lib and daemon"    OFF )
# ---------------------------------------------------------------------

if (BUILD_OSADAPTOR)
    message(STATUS "TinyNetFlow build sub lib [osadaptor]: ON")
    add_subdirectory(netflow/OSAdaptor)
    add_subdirectory(examples/osadaptor)
else ()
    message(STATUS "TinyNetFlow build sub lib [osadaptor]: OFF")
endif ()

if (BUILD_COM)
    message(STATUS "TinyNetFlow build sub lib [com]: ON")
    add_subdirectory(netflow/com)
else ()
    message(STATUS "TinyNetFlow build sub lib [com]: OFF")
endif ()

if (BUILD_EM)
    message(STATUS "TinyNetFlow build sub lib [em]: ON")
    add_subdirectory(netflow/em)
else ()
    message(STATUS "TinyNetFlow build sub lib [em]: OFF")
endif ()



#set(SPDLOG_CMAKE_CONFIG_DIR ${SPDLOG_BUILD_DIR})
#set(CMAKE_PREFIX_PATH ${SPDLOG_CMAKE_CONFIG_DIR})


