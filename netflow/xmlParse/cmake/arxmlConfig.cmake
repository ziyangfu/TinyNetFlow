# 注意： 下述路径是在install文件夹下的相对路径

# .
# ├── apps
# ├── daemon
# ├── etc
# ├── include
# └── lib
#     └── cmake
#         └── arxmlConfig.cmake

# get_filename_component: 这是一个 CMake 命令，用于从文件路径中提取特定部分。
# ${CMAKE_CURRENT_LIST_FILE}: 这是一个预定义的 CMake 变量，表示当前正在处理的 CMake 文件的完整路径。
# PATH: 这是指定要提取的部分，这里是文件的目录路径
# set library name
set(LIB_LIBRARIES "arxml")
set(LIB_OS_PLATFORM "Linux")

get_filename_component(LIB_CMAKE_DIR "arxmlConfig.cmake" PATH)
set(LIB_INCLUDE_DIRS "${LIB_CMAKE_DIR}/../../../include/${LIB_LIBRARIES}")

include("${LIB_CMAKE_DIR}/arxmlTargets.cmake")  # 这个文件是自动生成的
find_package(spdlog REQUIRED)