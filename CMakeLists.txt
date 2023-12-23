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
#      TinyNetFlow 顶层CMakeLists文件
# -----------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.16)
project(TinyNetFlow)

# OS部分，目前仅支持Linux
if (NOT DEFINED OS_PLATFORM)
    string(TOLOWER ${CMAKE_SYSTEM_NAME} OS_PLATFORM)
endif ()
if (OS_PLATFORM MATCHES "linux")
    include(GNUInstallDirs)  # for CMAKE_INSTALL_INCLUDEDIR ......
    INCLUDE(CheckFunctionExists) # for command check_function_exists ......
    message(STATUS "TinyNetFlow OS platform : Linux")
else ()
    message(SEND_ERROR "TinyNetFlow only support Linux platform now")
endif ()
# third_party
# spdlog
include_directories("${PROJECT_SOURCE_DIR}/third_party/spdlog/include/")
set(SPDLOG_CMAKE_CONFIG_DIR ${PROJECT_SOURCE_DIR}/third_party/spdlog/build)
set(CMAKE_PREFIX_PATH ${SPDLOG_CMAKE_CONFIG_DIR})
find_package(spdlog REQUIRED)
if (spdlog_FOUND)
    message(STATUS "spdlog found: ${spdlog_FOUND}")
endif ()

# 设置TinyNetFlow安装路径
set(NET_FLOW_DEFAULT_INSTALL_DIR ${PROJECT_BINARY_DIR}/install)

# 静态链接库
add_subdirectory(netflow/OSAdaptor)
# add_subdirectory(netflow/Com)
# add_subdirectory(netflow/Log)


