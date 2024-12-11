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

message(STATUS "TinyNetFlow OSAdaptor lib")
project(
        TinyNetFlow-OSAdaptor
        LANGUAGES CXX
        VERSION "0.0.1"
)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 这部分定义用户自定义的选项，包括自定义文件夹等
set(LIBRARY_NAME osadaptor)
set(INSTALL_INCLUDE_DIRECTORY ${CMAKE_INSTALL_INCLUDEDIR})  # ${CMAKE_INSTALL_PREFIX}/include
set(INSTALL_LIB_DIRECTORY ${CMAKE_INSTALL_LIBDIR})          # ${CMAKE_INSTALL_PREFIX}/lib
set(INSTALL_LIB_CMAKE_DIR ${INSTALL_LIB_DIRECTORY}/cmake)   # ${CMAKE_INSTALL_PREFIX}/lib/cmake
# 用户设置选项
message(STATUS "---------------------------------------------------")
option(INCLUDE_IPC "is include IPC" OFF)
option(INCLUDE_NET "is include NET" ON)
option(INCLUDE_PROCESS "is include process" OFF)

option(BUILD_EXAMPLES "build examples" OFF)
option(BUILD_TESTS "build unittest" OFF)
option(BUILD_DOXYGEN "documentation generation" OFF)
message(STATUS "---------------------------------------------------")

# 设置 Debug 配置的属性
set_target_properties(${LIBRARY_NAME} PROPERTIES
        DEBUG_POSTFIX "_d"  # 可选：为 Debug 构建添加后缀
        POSITION_INDEPENDENT_CODE ON
)
# 设置 Release 配置的属性
set_target_properties(${LIBRARY_NAME} PROPERTIES
        RELEASE_POSTFIX ""  # 可选：为 Release 构建添加后缀
        POSITION_INDEPENDENT_CODE ON
)
# 根据配置设置不同的编译选项, cmake -DCMAKE_BUILD_TYPE=debug
target_compile_options(osadaptor PRIVATE
        $<$<CONFIG:Debug>:              # 调试信息
                            -O0         # 不优化
                            -g
                            -Wall
                            -Wextra
                            # ......
                        >
        $<$<CONFIG:Release>: -O2>       # 编译器 O2 优化
)

find_package(spdlog)  # not required
if (spdlog_FOUND)
    message(STATUS "spdlog found: ${spdlog_FOUND}")
    include_directories("${PROJECT_SOURCE_DIR}/third_party/spdlog/include/")
else ()
    message(STATUS "spdlog lib do not find, using spdlog header")
    include_directories("${PROJECT_SOURCE_DIR}/third_party/spdlog/include/")
endif ()
# 添加线程库
find_package(Threads REQUIRED)
if (Thread_FOUND)
    message(STATUS "Threads found : ${Threads_FOUND}")
    target_link_libraries(${LIBRARY_NAME} ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(${LIBRARY_NAME} "rt")
else ()
    message(SEND_ERROR "Threads is necessary for osadaptor, please check")
endif ()

# 查找accept4系统调用是否存在，封装socket时，使用的accept4
check_function_exists(accept4 ACCEPT4_OK)
if (NOT ACCEPT4_OK)
    add_definitions(-DNO_ACCEPT4)
    message(STATUS "accept4 do not exist in current platform, will use accept ")
endif ()

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

file(GLOB_RECURSE LIB_OS_ADAPTOR_SRCS
                    src/IO/reactor/*.cpp
                    src/time/*.cpp
)
file(GLOB_RECURSE LIB_OS_ADAPTOR_HEADERS
                    include/IO/reactor/*.h
                    include/time/*.h
)
if (${INCLUDE_IPC})
    file(APPEND GLOB_RECURSE LIB_OS_ADAPTOR_SRCS src/IO/ipc/*.cpp)
    file(APPEND GLOB_RECURSE LIB_OS_ADAPTOR_HEADERS include/IO/ipc/*.h)
endif ()
if (${INCLUDE_NET})
    file(APPEND GLOB_RECURSE LIB_OS_ADAPTOR_SRCS src/IO/net/*.cpp)
    file(APPEND GLOB_RECURSE LIB_OS_ADAPTOR_HEADERS include/IO/net/*.h)
endif ()


add_library(${LIBRARY_NAME} STATIC ${LIB_OS_ADAPTOR_SRCS})
# add_library(${LIBRARY_NAME} STATIC ${LIB_OS_ADAPTOR_SRCS} ${LIB_OS_ADAPTOR_HEADERS})

# 设置目标属性,设置了库的包含目录
target_include_directories(${LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIRECTORY}>
)
# 安装部分，使用 make install 进行安装
install(
        TARGETS ${LIBRARY_NAME}
        EXPORT "${LIBRARY_NAME}Targets"
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
        COMPONENT dev
)


# 安装静态链接库
install(TARGETS ${LIBRARY_NAME}
                EXPORT "${LIBRARY_NAME}Targets"
                ARCHIVE DESTINATION ${INSTALL_LIB_DIRECTORY}/
                COMPONENT dev
)
# 安装头文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${INSTALL_INCLUDE_DIRECTORY}
        COMPONENT dev
)

# 导出目标到一个文件
install(TARGETS ${LIBRARY_NAME}
        EXPORT ${LIBRARY_NAME}Targets
        LIBRARY DESTINATION ${INSTALL_LIB_DIRECTORY}
        ARCHIVE DESTINATION ${INSTALL_LIB_DIRECTORY}
        INCLUDES DESTINATION include)

# 创建导出文件
install(EXPORT ${LIBRARY_NAME}Targets
        FILE ${LIBRARY_NAME}Targets.cmake
        DESTINATION ${INSTALL_LIB_CMAKE_DIR}/${LIBRARY_NAME})

# 安装头文件
install(DIRECTORY include/
        DESTINATION ${INSTALL_INCLUDE_DIRECTORY}/)



