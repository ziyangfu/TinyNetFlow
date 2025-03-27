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
# -----------------------------------------------------------------------------------------
set(IS_USE_Cpp20 0)
if (${IS_USE_Cpp20})
    # 设置使用clang编译器（目前开发机为clang-18， ubuntu20.04），并使用LLVM的libc++标准库
    # 指定 C++ 编译器为 clang++
    set(CMAKE_CXX_COMPILER "clang++")
    # 设置 C++ 标准库为 libc++
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    # 确保链接时使用 libc++
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++ -lc++abi")
    #set(CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "/lib/llvm-18/include/c++/v1")  # 替换为实际路径
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    message(STATUS "TinyNetFlow build with C++ standard: ${CMAKE_CXX_STANDARD}")
else ()
    # C++17
    # 默认使用gcc编译器（目前开发机为gcc-9， ubuntu20.04），并使用GNU的libstdc++标准库
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    message(STATUS "TinyNetFlow build with C++ standard: ${CMAKE_CXX_STANDARD}")
endif ()

project(TinyNetFlow
        LANGUAGES CXX
        VERSION 0.0.1
)

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

# 检查软件发行版是否为ubuntu
# 检查软件发行版
function(check_ubuntu_os)
    # 定义缓存变量存储检查结果
    set(IS_UBUNTU FALSE CACHE BOOL "Is Ubuntu OS")
    # 方法1：通过 /etc/os-release 检查
    execute_process(
            COMMAND cat /etc/os-release
            COMMAND grep -q "ID=ubuntu"
            RESULT_VARIABLE result_os_release
            OUTPUT_QUIET
            ERROR_QUIET
    )
    # 方法2：通过 /etc/lsb-release 检查（备用）
    execute_process(
            COMMAND cat /etc/lsb-release
            COMMAND grep -q "DISTRIB_ID=Ubuntu"
            RESULT_VARIABLE result_lsb_release
            OUTPUT_QUIET
            ERROR_QUIET
    )
    # 任一检查成功即判定为 Ubuntu
    if(result_os_release EQUAL 0 OR result_lsb_release EQUAL 0)
        set(IS_UBUNTU TRUE PARENT_SCOPE)
        set(IS_UBUNTU TRUE CACHE BOOL "Is Ubuntu OS" FORCE)
    endif()
endfunction()

check_ubuntu_os()

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
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -fno-omit-frame-pointer")  # 打开栈指针

# 设置Release模式的编译选项
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# ---------------------------------------------------------------------
# USDT/uprobe支持，可以使用 ksight/MagicEyes进行 netflow_APP-middleware-kernel三位一体观测
option(BPF_TRACE_ENABLE "Enable BPF tracing support" ON)
# ---------------------------------------------------------------------
if(BPF_TRACE_ENABLE)
    if (IS_UBUNTU)
        message(STATUS "Linux distribution: Ubuntu")
        # 检查 systemtap-sdt 是否安装
        execute_process(
                COMMAND dpkg -s systemtap-sdt-dev
                RESULT_VARIABLE result
                OUTPUT_QUIET ERROR_QUIET
        )
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "systemtap-sdt-dev package required for BPF tracing")
        endif()
    endif ()
    # 添加全局定义
    add_compile_definitions(BPF_TRACE)       # 方式1：CMake 3.12+
    #target_compile_definitions(osadaptor PUBLIC BPF_TRACE)  # 方式2：针对特定目标
    # 添加头文件路径
    include_directories(${SYSTEMTAP_SDT_INCLUDE_DIRS})
    message(STATUS "BPF tracing enabled")
else()
    message(STATUS "BPF tracing disabled")
endif()

# 设置默认安装目录 <build_dir>/install
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)
message(STATUS "project install dir : ${CMAKE_INSTALL_PREFIX}")
# ---------------------------------------------------------------------
option(BUILD_OSADAPTOR  "is build osadaptor lib"        ON  )
option(BUILD_COM        "is build com lib and daemon"   ON )
option(BUILD_EM         "is build em lib and daemon"    ON )
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

# var BUILD_OSA_TESTS defined in osadaptor
if (${BUILD_OSADAPTOR} AND ${BUILD_OSA_TESTS})
    add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
endif ()



#set(SPDLOG_CMAKE_CONFIG_DIR ${SPDLOG_BUILD_DIR})
#set(CMAKE_PREFIX_PATH ${SPDLOG_CMAKE_CONFIG_DIR})