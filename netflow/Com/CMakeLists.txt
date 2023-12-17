# \copyright
# Copyright (c) 2023 by the TinyNetFlow project authors. All Rights Reserved.
#
# This file is open source software, licensed to you under the ter；ms
# of the Apache License, Version 2.0 (the "License").  See the NOTICE file
# distributed with this work for additional information regarding copyright
# ownership.  You may not use this file except in compliance with the License.
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
# -----------------------------------------------------------------------------------------
# \brief
#      OSAdaptor 操作系统适配层库的顶层CMakeLists文件
# -----------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16)

project(
        TinyNetFlow-OSAdaptor
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
include_directories("")

file(GLOB_RECURSE LIB_OS_ADAPTOR_SRCS
        ${CMAKE_SOURCE_DIR}/IO/*.cpp
        ${CMAKE_SOURCE_DIR}/process/*.cpp
        ${CMAKE_SOURCE_DIR}/time/*.cpp
)

add_library(${LIBRARY_NAME} STATIC ${LIB_OS_ADAPTOR_SRCS})

find_package(Threads REQUIRED)
message(STATUS "Threads found : ${Threads_FOUND}")
target_link_libraries(${LIBRARY_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${LIBRARY_NAME} "rt")

file(GLOB_RECURSE LIB_OS_ADAPTOR_HEADERS
        ${CMAKE_SOURCE_DIR}/IO/*.h
        ${CMAKE_SOURCE_DIR}/process/*.h
        ${CMAKE_SOURCE_DIR}/time/*.h
)
target_include_directories(${LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${LIB_OS_ADAPTOR_HEADERS}>
        $<INSTALL_DIRECTORY>:${INSTALL_INCLUDE_DIRECTORY}
)
# 安装部分，使用 make install 进行安装
# 安装静态链接库
install(TARGETS ${LIBRARY_NAME}
                EXPORT "${LIBRARY_NAME}Targets"
                ARCHIVE DESTINATION ${INSTALL_LIB_DIRECTORY}
                COMPONENT dev
)
# 安装头文件
install(DIRECTORY ${LIB_OS_ADAPTOR_HEADERS}
        DESTINATION ${INSTALL_INCLUDE_DIRECTORY}
        COMPONENT dev
)

# 导出
export(TARGETS ${LIBRARY_NAME} FILE "${PROJECT_BINARY_DIR}/${LIBRARY_NAME}Targets.cmake")
export(PACKAGE ${LIBRARY_NAME})

foreach (p LIB_INCLUDE CMAKE)

endforeach ()