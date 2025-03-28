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
#      TinyNetFlow 通信模块的顶层CMakeLists文件，生成静态链接库 libcom，守护进程 someipd 等
# -----------------------------------------------------------------------------------------
message(STATUS "will build TinyNetFlow com module")
set(NETFLOW_MODULE_NAME com)
set(NETFLOW_MODULE_VERSION "0.0.1")
# -----------------------------------------------------------------------------------------
option(BUILD_LIB_COM "is build libcom" ON)
option(BUILD_SOMEIP_DAEMON "is build someipd" OFF)
# -----------------------------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/include)
include_directories("${PROJECT_SOURCE_DIR}/third_party/spdlog/include/")
# 后续使用 find_package(osadaptor REQUIRED)
include_directories(${PROJECT_SOURCE_DIR}/netflow/OSAdaptor/include)

if (${BUILD_LIB_COM})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)
endif ()
if (${BUILD_SOMEIP_DAEMON})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/someipd)
endif ()

# install etc and script, if it is exist
# install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/etc/emConfig.json
#        DESTINATION ${CMAKE_INSTALL_PREFIX}/etc/)








