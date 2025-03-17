message(STATUS "\t part of TinyNetFlow execmd daemon and execManagerClient lib")
set(EM_DAEMON_NAME execmd)
set(LIBRARY_NAME execManagerClient)
set(LIBRARY_VERSION "0.0.1")

set(ARG_PARSE_DIR ${CMAKE_SOURCE_DIR}/third_party/argparse/include)
include_directories(${ARG_PARSE_DIR})  # cmd parser
# ${CMAKE_INSTALL_PREFIX}/include/LIBRARY_NAME
set(INSTALL_INCLUDE_DIRECTORY ${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_NAME})
set(INSTALL_LIB_DIRECTORY ${CMAKE_INSTALL_LIBDIR})          # ${CMAKE_INSTALL_PREFIX}/lib
set(INSTALL_LIB_CMAKE_DIR ${INSTALL_LIB_DIRECTORY}/cmake)   # ${CMAKE_INSTALL_PREFIX}/lib/cmake
set(INSTALL_EXEC_DAEMON_DIR ${CMAKE_INSTALL_PREFIX}/daemon)
# find_package(osadaptor)
include_directories("${PROJECT_SOURCE_DIR}/third_party/spdlog/include/")

file(GLOB_RECURSE EXECD_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/${EM_DAEMON_NAME}/*.cpp)
file(GLOB_RECURSE EXECD_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/${EM_DAEMON_NAME}/*.h)
add_executable(${EM_DAEMON_NAME} ${EXECD_SRCS})
target_link_libraries(${EM_DAEMON_NAME} osadaptor)
# 安装execmd
install(TARGETS ${EM_DAEMON_NAME}
        RUNTIME DESTINATION ${INSTALL_EXEC_DAEMON_DIR})

# part of execManagerClient lib
file(GLOB_RECURSE EM_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/*.cpp)
file(GLOB_RECURSE EM_LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/lib/include/*.h)
add_library(${LIBRARY_NAME} STATIC ${EM_LIB_SRCS})

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
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/include/
        DESTINATION ${INSTALL_INCLUDE_DIRECTORY}/)

# 安装cmake配置文件
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib/cmake/${LIBRARY_NAME}Config.cmake
        DESTINATION ${INSTALL_LIB_CMAKE_DIR}/${LIBRARY_NAME})
# 安装etc文件
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/etc/emConfig.json
        DESTINATION ${CMAKE_INSTALL_PREFIX}/etc/)

