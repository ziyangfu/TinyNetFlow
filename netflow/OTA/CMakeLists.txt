# \copyright
# Copyright (c) 2025 by the TinyNetFlow project authors. All Rights Reserved.
#
# This file is open source software, licensed to you under the terms
# of the Apache License, Version 2.0 (the "License").  See the NOTICE file
# distributed with this work for additional information regarding copyright
# ownership.  You may not use this file except in compliance with the License.
# You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
# -----------------------------------------------------------------------------------------
# \brief
#      TinyNetFlow 诊断及OTA模块的顶层CMakeLists文件，生成一系列的静态链接库如：libDoIP等，
#                  可能还有一些可执行文件 remoteDiagd， dltd等
# -----------------------------------------------------------------------------------------
message(STATUS "\t will build TinyNetFlow OTA module")
set(NETFLOW_MODULE_NAME OTA)
set(NETFLOW_MODULE_VERSION "0.0.1")
# -----------------------------------------------------------------------------------------
# user option
option(BUILD_DLT_DAEMON         "is build DLT daemon"                               ON)
option(ADD_DLT_VIEWER_EXEC      "is add DLT viewer appimage to install tools dir "  ON)
option(BUILD_DOIP_LIB           "is build DoIP lib"                                 ON)
option(BUILD_LLM_OTA_TOOLS      "is build LLM OTA tools"                            OFF)
option(BUILD_REMOTE_DIAG_DAEMON "is build remote diag daemon"                       OFF)
# -----------------------------------------------------------------------------------------
include_directories("${PROJECT_SOURCE_DIR}/third_party/spdlog/include/")

if (${BUILD_DLT_DAEMON})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dltDaemon)
endif ()
if (${ADD_DLT_VIEWER_EXEC})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dltViewer)
endif ()
if (${BUILD_DOIP_LIB})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/DoIP)
endif ()
if (${BUILD_LLM_OTA_TOOLS})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/LLM)
endif ()
if (${BUILD_REMOTE_DIAG_DAEMON})
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/remoteDiag)
endif ()












