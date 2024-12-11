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
#      TinyNetFlow 通信模块的顶层CMakeLists文件，生成静态链接库 TinyNetFlow-Com
# -----------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)

project(
        TinyNetFlow-Com
        LANGUAGES CXX
        VERSION "0.0.1"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED OS_PLATFORM)
    string(TOLOWER ${CMAKE_SYSTEM_NAME} OS_PLATFORM)
endif ()
if (OS_PLATFORM MATCHES "Linux")
    message(STATUS "TinyNetFlow OS platform is Linux")
else ()
    message(SEND_ERROR "TinyNetFlow only support Linux platform now")
endif ()

message(STATUS "--------------- 用户设置选项 ----------------------------------")
# 这部分定义用户自定义的选项，包括自定义文件夹等
message(STATUS "-------------------------------------------------------------")

set(LIB_VERSION 0.0.1)
