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
#      TinyNetFlow xml(arxml)解析库的顶层CMakeLists文件，生成静态链接库 libarxml
# -----------------------------------------------------------------------------------------

message(STATUS "\t part of TinyNetFlow arxml parser lib")
# 这部分定义用户自定义的选项，包括自定义文件夹等
set(LIBRARY_NAME arxml)
set(LIBRARY_VERSION "0.0.1")
# ${CMAKE_INSTALL_PREFIX}/include/arxml
set(INSTALL_INCLUDE_DIRECTORY ${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_NAME})
set(INSTALL_LIB_DIRECTORY ${CMAKE_INSTALL_LIBDIR})          # ${CMAKE_INSTALL_PREFIX}/lib
set(INSTALL_LIB_CMAKE_DIR ${INSTALL_LIB_DIRECTORY}/cmake)   # ${CMAKE_INSTALL_PREFIX}/lib/cmake

# include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# 用户设置选项
# ---------------------------------------------------------------------
# 是否同时编译spdlog库，若编译，则使用库，并将库安装在install/third_party/spdlog
option(BUILD_SPDLOG "build spdlog static lib" OFF)
# ---------------------------------------------------------------------
# ---------------------------------------------------------------------
option(BUILD_ARXML_EXAMPLES   "build libarxml examples"                OFF)
option(BUILD_ARXML_TESTS      "build libarxml unittest"                ON)
option(BUILD_ARXML_DOXYGEN    "libarxml documentation generation"      OFF)
# ---------------------------------------------------------------------
if (${BUILD_ARXML_EXAMPLES})
    message(STATUS "\t build arxml examples: ${BUILD_ARXML_EXAMPLES}")
    set(CACHE BUILD_ARXML_EXAMPLES 1)
elseif (${BUILD_ARXML_TESTS})
    message(STATUS "\t build arxml unit tests: ${BUILD_ARXML_TESTS}")
    set(CACHE BUILD_ARXML_TESTS 1)
elseif (${BUILD_ARXML_DOXYGEN})
    message(STATUS "\t libarxml documentation generation: ${BUILD_ARXML_DOXYGEN}")
    set(CACHE BUILD_ARXML_DOXYGEN 1)
endif ()
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

# -------------------------------------------------------------------------------------
# part of source code
file(GLOB_RECURSE LIB_ARXML_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE LIB_ARXML_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB PUGIXML_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/pugixml/pugixml.cpp)
file(GLOB PUGIXML_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/pugixml/*.hpp)

set(LIB_ARXML_ALL_SRCS  ${LIB_ARXML_HEADERS}
                        ${LIB_ARXML_SRCS}
                        ${PUGIXML_HEADERS}
                        ${PUGIXML_SRCS})
#----------------------------------------------------------------------------------------

add_library(${LIBRARY_NAME} STATIC ${LIB_ARXML_ALL_SRCS})

set(CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX}/third_party/spdlog)
include_directories("${PROJECT_SOURCE_DIR}/third_party/spdlog/include/")

# 添加线程库
find_package(Threads REQUIRED)
if (Threads_FOUND)
    message(STATUS "\t Threads found : ${Threads_FOUND}")
    target_link_libraries(${LIBRARY_NAME} ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(${LIBRARY_NAME} "rt")
else ()
    message(SEND_ERROR "\t Threads is necessary for arxml, please check")
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


