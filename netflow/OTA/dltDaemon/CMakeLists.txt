# 控制DLT-Daemon的是否编译生成
# [DLT-daemon](https://github.com/COVESA/dlt-daemon)
message(STATUS "\t will include DLT daemon, it is based on (or copy from) COVESA dlt-daemon")

set(DLT_DAEMON_PROJECT_DIR ${PROJECT_SOURCE_DIR}/third_party/dlt-daemon)
set(DLT_DAEMON_BUILD_DIR ${PROJECT_BINARY_DIR}/dlt_daemon_build)   # <>/build/dlt_daemon_build
set(DLT_DAEMON_INSTALL_TEMP_DIR ${DLT_DAEMON_BUILD_DIR}/dlt_daemon_install_temp)

file(MAKE_DIRECTORY ${DLT_DAEMON_BUILD_DIR})
# 添加第三方指令 custom command 编译 DLT Daemon
add_custom_command(
        OUTPUT ${DLT_DAEMON_INSTALL_TEMP_DIR}/lib/libdlt.so
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build_dlt_daemon.sh ${DLT_DAEMON_PROJECT_DIR}
        ${DLT_DAEMON_BUILD_DIR} ${DLT_DAEMON_INSTALL_TEMP_DIR}
        COMMENT "Building DLT Daemon and install to temp dir"
)
# 添加自定义目标来确保编译命令被执行
add_custom_target(build_dlt_daemon ALL
        DEPENDS ${DLT_DAEMON_INSTALL_TEMP_DIR}/lib/libdlt.so
)

# install
# 1. 将DLT_DAEMON_INSTALL_TEMP_DIR/include下的所有文件，复制到 <CMAKE_INSTALL_PREFIX>/include下面
# 2. 将 DLT_DAEMON_INSTALL_TEMP_DIR/lib下面的libdlt* 复制到<CMAKE_INSTALL_PREFIX>/lib下面
# 3. 将 DLT_DAEMON_INSTALL_TEMP_DIR/lib/cmake下面的automotive-dlt文件夹 复制到<CMAKE_INSTALL_PREFIX>/lib/cmake下面
install(DIRECTORY ${DLT_DAEMON_INSTALL_TEMP_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        FILES_MATCHING PATTERN "*.h"
        PATTERN ".*" EXCLUDE
)

file(GLOB LIBDLT_ALL ${DLT_DAEMON_INSTALL_TEMP_DIR}/lib/libdlt*)
install(FILES ${LIBDLT_ALL}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

install(DIRECTORY ${DLT_DAEMON_INSTALL_TEMP_DIR}/lib/cmake/automotive-dlt/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/automotive-dlt
)