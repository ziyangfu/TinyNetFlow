# 注意： 下述路径是在install文件夹下的相对路径

# .
# ├── apps
# ├── daemon
# ├── etc
# ├── include
# └── lib
#     └── cmake
#         └── osadaptorConfig.cmake

# get_filename_component: 这是一个 CMake 命令，用于从文件路径中提取特定部分。
# OSADATOR_CMAKE_DIR: 这是输出变量，将存储提取的路径。
# ${CMAKE_CURRENT_LIST_FILE}: 这是一个预定义的 CMake 变量，表示当前正在处理的 CMake 文件的完整路径。
# PATH: 这是指定要提取的部分，这里是文件的目录路径
get_filename_component(OSADATOR_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(OSADATOR_INCLUDE_DIRS "${OSADATOR_CMAKE_DIR}/../../include")
# set osadaptor library name
set(OSADATOR_LIBRARIES "osadaptor")
set(OSADAPTOR_PLATFORM "Linux")
include("${OSADATOR_CMAKE_DIR}/osadaptorTargets.cmake")  # 这个文件是自动生成的
find_package(spdlog REQUIRED)