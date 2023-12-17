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
#      OSAdaptor 操作系统适配层库的顶层CMakeLists文件
# -----------------------------------------------------------------------------------------

message(STATUS "TinyNetFlow OSAdaptor build")
project(
        TinyNetFlow-OSAdaptor
        LANGUAGES CXX
        VERSION "0.0.1"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


message(STATUS "用户设置选项: ------------------------")
# 这部分定义用户自定义的选项，包括自定义文件夹等
set(LIBRARY_NAME OSAdaptor)
set(INSTALL_INCLUDE_DIRECTORY ${CMAKE_INSTALL_INCLUDEDIR})  # include
set(INSTALL_LIB_DIRECTORY ${CMAKE_INSTALL_LIBDIR})   # lib
set(INSTALL_LIB_CMAKE_DIR ${INSTALL_LIB_DIRECTORY}/cmake)  # <...>/lib/cmake

option(BUILD_EXAMPLES "build examples" ON)
option(BUILD_TESTS "build unittest" OFF)
option(BUILD_DOXYGEN "documentation generation" OFF)

set(${CMAKE_CXX_FLAGS_DEBUG} "-O0")     # 不优化
set(${CMAKE_CXX_FLAGS_RELEASE} "-O2")   # 编译器 O2 优化
# 设置编译选项
set(${CMAKE_CXX_FLAGS}
        -g
        -Wall
        -Wextra
        # ......
)


# TODO 设置安装路径

message(STATUS "--------------------------------")

# ---------------------------------------------------------------------------
set(OS_ADAPTOR_LIB_VERSION 0.0.1)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})  # [<>/OSAdaptor]/ ...

file(GLOB_RECURSE LIB_OS_ADAPTOR_SRCS
        include/IO/*.cpp
        include/process/*.cpp
        include/time/*.cpp
)

add_library(${LIBRARY_NAME} STATIC ${LIB_OS_ADAPTOR_SRCS})
# 添加线程库
find_package(Threads REQUIRED)
message(STATUS "Threads found : ${Threads_FOUND}")
target_link_libraries(${LIBRARY_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${LIBRARY_NAME} "rt")

file(GLOB_RECURSE LIB_OS_ADAPTOR_HEADERS
        include/IO/*.h
        include/process/*.h
        include/time/*.h
)

target_include_directories(${LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>   # TODO： 文件夹重新弄
        $<INSTALL_DIRECTORY>:${INSTALL_INCLUDE_DIRECTORY}
)
# 安装部分，使用 make install 进行安装
# 安装静态链接库
install(TARGETS ${LIBRARY_NAME}
                EXPORT "${LIBRARY_NAME}Targets"
                ARCHIVE DESTINATION ${INSTALL_LIB_DIRECTORY}/
                COMPONENT dev
)
# 安装头文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DESTINATION ${INSTALL_INCLUDE_DIRECTORY}
        COMPONENT dev
)

# 导出
# export(TARGETS ${LIBRARY_NAME} FILE "${PROJECT_BINARY_DIR}/${LIBRARY_NAME}Targets.cmake")
# export(PACKAGE ${LIBRARY_NAME})

# foreach (p LIB_INCLUDE CMAKE)

# endforeach ()

# ----------------------------------------------------------------------------------------------

if (BUILD_EXAMPLES)
    message(STATUS "build OS Adaptor examples : ON")
    # add_subdirectory(func/examples)  # TODO 增加examples文件夹
else ()
    message(STATUS "build OS Adaptor examples: OFF")
endif ()

if (BUILD_TESTS)
    message(STATUS "build OS Adaptor tests : ON")
    # add_subdirectory(func/tests) # TODO 增加tests文件夹
else ()
    message(STATUS "build OS Adaptor tests: OFF")
endif ()

if (BUILD_DOXYGEN)
    message(STATUS "documentation generation: ON")
    # add_subdirectory(func/doc)
else ()
    message(STATUS "documentation generation: OFF")
endif ()