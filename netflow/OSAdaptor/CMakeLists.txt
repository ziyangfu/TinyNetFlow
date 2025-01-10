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

message(STATUS "\t part of TinyNetFlow OSAdaptor lib")
# 这部分定义用户自定义的选项，包括自定义文件夹等
set(LIBRARY_NAME osadaptor)
set(LIBRARY_VERSION "0.0.1")
set(INSTALL_INCLUDE_DIRECTORY ${CMAKE_INSTALL_INCLUDEDIR})  # ${CMAKE_INSTALL_PREFIX}/include
set(INSTALL_LIB_DIRECTORY ${CMAKE_INSTALL_LIBDIR})          # ${CMAKE_INSTALL_PREFIX}/lib
set(INSTALL_LIB_CMAKE_DIR ${INSTALL_LIB_DIRECTORY}/cmake)   # ${CMAKE_INSTALL_PREFIX}/lib/cmake
# 用户设置选项
# 是否同时编译spdlog库，若编译，则使用库，并将库安装在install/third_party/spdlog
option(BUILD_SPDLOG "build spdlog static lib" OFF)

option(INCLUDE_IPC "is include IPC" ON)
option(INCLUDE_NET "is include NET" ON)
option(INCLUDE_PROCESS "is include process" OFF)

option(BUILD_EXAMPLES "build examples" ON)
option(BUILD_TESTS "build unittest" OFF)
option(BUILD_DOXYGEN "documentation generation" OFF)

if (BUILD_SPDLOG)
    add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/spdlog/libspdlog.a
            COMMAND cmake -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/install/third_party/spdlog
                    ${PROJECT_SOURCE_DIR}/third_party/spdlog -B ${PROJECT_BINARY_DIR}/spdlog
            COMMAND cmake --build ${PROJECT_BINARY_DIR}/spdlog --target spdlog
            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
            COMMENT "Building spdlog static library"
    )
    # 添加一个自定义目标来依赖这个命令
    add_custom_target(build_spdlog ALL DEPENDS ${PROJECT_BINARY_DIR}/spdlog/libspdlog.a)

    # 在 make install 时执行安装 spdlog 的操作
    install(CODE "
    execute_process(COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/spdlog --target install
                    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
                    RESULT_VARIABLE result)
    if(result)
        message(FATAL_ERROR \"Failed to install spdlog\")
    endif()
")
endif ()
# 查找accept4系统调用是否存在，封装socket时，使用的accept4
check_function_exists(accept4 ACCEPT4_OK)
if (NOT ACCEPT4_OK)
    add_definitions(-DNO_ACCEPT4)
    message(STATUS "\t accept4 do not exist in current platform, will use accept ")
endif ()

if (BUILD_EXAMPLES)
    message(STATUS "\t build OS Adaptor examples : ON")
    # add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../examples/${LIBRARY_NAME})
else ()
    message(STATUS "\t build OS Adaptor examples: OFF")
endif ()

if (BUILD_TESTS)
    message(STATUS "\t build OS Adaptor tests : ON")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../tests/${LIBRARY_NAME})
else ()
    message(STATUS "\t build OS Adaptor tests: OFF")
endif ()

if (BUILD_DOXYGEN)
    message(STATUS "\t documentation generation: ON")
    # add_subdirectory(func/doc)
else ()
    message(STATUS "\t documentation generation: OFF")
endif ()

file(GLOB_RECURSE LIB_OS_ADAPTOR_REACTOR_SRCS
                    src/IO/reactor/*.cpp
                    src/time/*.cpp
)
file(GLOB_RECURSE LIB_OS_ADAPTOR_REACTOR_HEADERS
                    include/IO/reactor/*.h
                    include/time/*.h
)
if (${INCLUDE_IPC})
    message(STATUS "include IPC source code")
    file(GLOB_RECURSE LIB_OS_ADAPTOR_IPC_SRCS src/IO/ipc/uds/*.cpp)
    file(GLOB_RECURSE LIB_OS_ADAPTOR_IPC_HEADERS include/IO/ipc/uds/*.h)
else ()
    file(GLOB_RECURSE LIB_OS_ADAPTOR_IPC_SRCS "")
    file(GLOB_RECURSE LIB_OS_ADAPTOR_IPC_HEADERS "")
endif ()
if (${INCLUDE_NET})
    message(STATUS "include Net source code")
    file(GLOB_RECURSE LIB_OS_ADAPTOR_NET_SRCS src/IO/net/*.cpp)
    file(GLOB_RECURSE LIB_OS_ADAPTOR_NET_HEADERS include/IO/net/*.h)
else ()
    file(GLOB_RECURSE LIB_OS_ADAPTOR_NET_SRCS "")
    file(GLOB_RECURSE LIB_OS_ADAPTOR_NET_HEADERS "")
endif ()

set(LIB_OS_ADAPTOR_SRCS ${LIB_OS_ADAPTOR_REACTOR_SRCS}
                        ${LIB_OS_ADAPTOR_IPC_SRCS}
                        ${LIB_OS_ADAPTOR_NET_SRCS})
add_library(${LIBRARY_NAME} STATIC ${LIB_OS_ADAPTOR_SRCS})
# add_library(${LIBRARY_NAME} STATIC ${LIB_OS_ADAPTOR_SRCS} ${LIB_OS_ADAPTOR_HEADERS})
set(CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX}/third_party/spdlog)
#find_package(spdlog)  # not required
#if (spdlog_FOUND)
#    message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
#    message(STATUS "\t spdlog found: ${spdlog_FOUND}")
#    target_link_libraries(${LIBRARY_NAME} spdlog)
#    target_include_directories(${LIBRARY_NAME} PRIVATE ${spdlog_INCLUDE_DIRS})
#    include_directories("${PROJECT_SOURCE_DIR}/third_party/spdlog/include/")
#else ()
#    message(STATUS "\t spdlog lib do not find, using spdlog header")
#    include_directories("${PROJECT_SOURCE_DIR}/third_party/spdlog/include/")
#endif ()
include_directories("${PROJECT_SOURCE_DIR}/third_party/spdlog/include/")

# 添加线程库
find_package(Threads REQUIRED)
if (Threads_FOUND)
    message(STATUS "\t Threads found : ${Threads_FOUND}")
    target_link_libraries(${LIBRARY_NAME} ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(${LIBRARY_NAME} "rt")
else ()
    message(SEND_ERROR "\t Threads is necessary for osadaptor, please check")
endif ()

# 设置目标属性,设置了库的包含目录
target_include_directories(${LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
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

# 安装配置文件
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${LIBRARY_NAME}Config.cmake
        DESTINATION ${INSTALL_LIB_CMAKE_DIR}/${LIBRARY_NAME})



# 知识积累：

# 在 make install 时执行安装 spdlog 的操作， 也可以写在单独的spdlog_install.cmake 脚本里
# install_spdlog.cmake
#    execute_process(COMMAND ${CMAKE_COMMAND} --build ${PROJECT_BINARY_DIR}/spdlog --target install
#            WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
#            RESULT_VARIABLE result)
#    if(result)
#        message(FATAL_ERROR "Failed to install spdlog")
#    endif()

# 当前cmake文件
# if (BUILD_SPDLOG)
#    message(STATUS "\t build spdlog static lib: ON")
#    add_custom_command(
#            ......
#            COMMENT "Building spdlog static library"
#    )
#    # 添加一个自定义目标来依赖这个命令
#    add_custom_target(build_spdlog ALL DEPENDS ${PROJECT_BINARY_DIR}/spdlog/libspdlog.a)
#    # 在 make install 时执行安装 spdlog 的操作
#    install(SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/install_spdlog.cmake)
#endif ()


# 构建spdlohg可以使用ExternalProject_Add函数
#ExternalProject_Add(<proj>
#  [PREFIX <prefix>]
#  [TMP_DIR <tmp_dir>]
#  [STAMP_DIR <stamp_dir>]
#  [DOWNLOAD_DIR <download_dir>]
#  [SOURCE_DIR <source_dir>]
#  [BINARY_DIR <binary_dir>]
#  [INSTALL_DIR <install_dir>]
#  [DOWNLOAD_COMMAND <download_command>]
#  [URL <url>...]
#  [URL_HASH <url_hash>]
#  [DOWNLOAD_NAME <name>]
#  [TIMEOUT <timeout>]
#  [HTTP_USERNAME <username>]
#  [HTTP_PASSWORD <password>]
#  [TLS_VERIFY <bool> | TLS_CAINFO <file>]
#  [CMAKE_COMMAND <cmake_command>]
#  [CONFIGURE_COMMAND <configure_command>]
#  [BUILD_COMMAND <build_command>]
#  [INSTALL_COMMAND <install_command>]
#  [LOG_DOWNLOAD <log_download>]
#  [LOG_UPDATE <log_update>]
#  [LOG_CONFIGURE <log_configure>]
#  [LOG_BUILD <log_build>]
#  [LOG_INSTALL <log_install>]
#  [BUILD_IN_SOURCE <bool>]
#  [CONFIGURE_HANDLED_BY_BUILD <bool>]
#  [BUILD_ALWAYS <bool>]
#  [UPDATE_DISCONNECTED <bool>]
#  [EXCLUDE_FROM_ALL <bool>]
#  [STEP_TARGETS <step-targets>...]
#  [COMMAND <command>...]
#)


