message(STATUS "will build DoIP protocol lib")
set(LIBRARY_NAME doip)
# -----------------------------------------------------------------------------------------
# user option
option(BUILD_DOIP_EXAMPLES         "Build all the samples."     ON )
option(BUILD_DOIP_TESTS            "Build all the tests"        ON )
option(BUILD_DOIP_DOXYGEN           "document generation"       ON )
# -----------------------------------------------------------------------------------------
# CMAKE_INSTALL_INCLUDEDIR定义在GNUInstallDirs里面
set(INSTALL_INCLUDE_DIRECTORY ${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_NAME})
set(INSTALL_LIB_DIRECTORY ${CMAKE_INSTALL_LIBDIR})          # ${CMAKE_INSTALL_PREFIX}/lib
set(INSTALL_LIB_CMAKE_DIR ${INSTALL_LIB_DIRECTORY}/cmake)   # ${CMAKE_INSTALL_PREFIX}/lib/cmake

find_package(Threads REQUIRED)
if (Threads_FOUND)
    message(STATUS "\t Threads found : ${Threads_FOUND}")
else ()
    message(SEND_ERROR "\t Threads is necessary for DoIP lib, please check")
endif ()
set(DoIP_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${DoIP_LIB_INCLUDE_DIR})
file(GLOB DoIP_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB DoIP_LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

add_library(${LIBRARY_NAME} STATIC ${DoIP_LIB_SRCS})
target_link_libraries(${LIBRARY_NAME} Threads::Threads)

#target_include_directories(${LIBRARY_NAME}
#  INTERFACE ${DoIP_LIB_INCLUDE_DIR}
#)

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

# 安装cmake配置文件
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${LIBRARY_NAME}Config.cmake
        DESTINATION ${INSTALL_LIB_CMAKE_DIR}/${LIBRARY_NAME})
