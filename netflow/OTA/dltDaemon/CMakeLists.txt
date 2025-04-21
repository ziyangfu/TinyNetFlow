# 控制DLT-Daemon的是否编译生成
# [DLT-daemon](https://github.com/COVESA/dlt-daemon)
message(STATUS "\t will include DLT daemon, it is based on (or copy from) COVESA dlt-daemon")
# 先执行dlt-daemon的安装命令，安装在 dlt_daemon_install_temp 文件夹
set(DLT_DAEMON_INSTALL_TEMP_DIR ${PROJECT_BINARY_DIR}/dlt_daemon_install_temp)
if (${BUILD_DLT_DAEMON})
    # 添加第三方指令 custom command 编译 DLT Daemon
endif ()

# install
# 复制install_temp中的内容，将库安装到 <install_dir>/lib/dltDaemon中