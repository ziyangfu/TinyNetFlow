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
#      静态链接库 libcom 顶层 CMakeLists.txt
# -----------------------------------------------------------------------------------------
message(STATUS "\t part of TinyNetFlow-COM libcom")
set(LIBRARY_NAME ${NETFLOW_MODULE_NAME})   # same with com module name
set(LIBRARY_VERSION "0.0.1")
set(INSTALL_INCLUDE_DIRECTORY ${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_NAME})
set(INSTALL_LIB_DIRECTORY ${CMAKE_INSTALL_LIBDIR})          # ${CMAKE_INSTALL_PREFIX}/lib
set(INSTALL_LIB_CMAKE_DIR ${INSTALL_LIB_DIRECTORY}/cmake)   # ${CMAKE_INSTALL_PREFIX}/lib/cmake
# --------------------------------------------------------------------------------------------
option(ADD_HTTP "is add http protocol?"                 ON  )
option(USING_HTTPS "is using HTTPS based on openssl"    ON  )  # must ADD_HTTP == ON
option(ADD_MQTT "is add MQTT protocol?"                 ON  )
option(ADD_SOMEIP "is add someip protocol?"             OFF )
# --------------------------------------------------------------------------------------------
# part of com lib
if (${ADD_HTTP})
    message(STATUS "    libcom: add http")
    file(GLOB COM_HTTP_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/http/*.cpp)
    file(GLOB COM_HTTP_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/protocol/http/*.h)
else ()
    set(COM_HTTP_SRCS "")
endif ()
if (${ADD_MQTT})
    message(STATUS "    libcom: add mqtt")
    file(GLOB COM_MQTT_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/mqtt/*.cpp)
    file(GLOB COM_MQTT_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/protocol/mqtt/*.h)
else ()
    set(COM_MQTT_SRCS "")
endif ()
if (${ADD_SOMEIP})
    add_subdirectory(lib/include/protocol/someip)
endif ()

if (${USING_HTTPS})
    if (!${ADD_HTTP})
        message(SEND_ERROR "https must enable http")
    endif ()
    find_package(OpenSSL QUIET)
    if(OPENSSL_FOUND)
        message(STATUS "Found OpenSSL ${OPENSSL_VERSION}， enable SSL wrapper")
        add_compile_definitions(ENABLE_OPENSSL)
        file(GLOB_RECURSE COM_SSL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/ssl/*.cpp)
        file(GLOB_RECURSE COM_SSL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/ssl/*.h)
    else ()
        set(COM_SSL_SRCS "")
    endif()

endif ()

set(COM_LIB_SRCS    ${COM_HTTP_SRCS}
                    ${COM_MQTT_SRCS}
                    ${COM_SSL_SRCS}
)

#message(STATUS "com include lib src: ${COM_LIB_SRCS}")
add_library(${LIBRARY_NAME} STATIC ${COM_LIB_SRCS})

# 设置目标属性,设置了库的包含目录
target_include_directories(${LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/include/>
        $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIRECTORY}>
)
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
target_compile_options(${LIBRARY_NAME} PRIVATE
        $<$<CONFIG:Debug>:              # 调试信息
        -O0         # 不优化
        -g
        -Wall
        -Wextra
        # ......
        >
        $<$<CONFIG:Release>: -O2>       # 编译器 O2 优化
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
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${INSTALL_INCLUDE_DIRECTORY}/)

# 安装cmake配置文件
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${LIBRARY_NAME}Config.cmake
        DESTINATION ${INSTALL_LIB_CMAKE_DIR}/${LIBRARY_NAME})