#!/bin/bash
# 脚本：execmd_init_systemd.sh
# 功能：初始化 execmd 程序的 systemd 服务
# 作者：fzy
# 创建时间：2024-12
# 使用范例：
# sudo ./execmd_init_systemd.sh --dir <...>/<build>/<install>
# 管理： systemctl status NetFlowExecmd.service


# 函数：显示帮助信息
show_help() {
    echo "Usage: $0 --dir <directory> [-h]"
    echo
    echo "Options:"
    echo "  --dir <directory>    TinyNetFlow install directory, for systemd service Working Directory."
    echo "  -h                  Show this help message."
    exit 0
}
# 检查是否具有root权限
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root. Please use sudo if in ubuntu/debian."
    exit 1
fi

# 检查是否提供了参数
if [[ "$#" -eq 0 ]]; then
    show_help
    exit 1
fi
# 解析参数
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --dir)
            NETFLOW_INSTALL_DIR="$2"
            shift 2
            ;;
        -h)
            show_help
            ;;
        *)
            echo "Unknown parameter passed: $1"
            show_help
            exit 1
            ;;
    esac
done
# 检查是否提供了 --dir 参数
if [[ -z "$NETFLOW_INSTALL_DIR" ]]; then
    echo "Error: --dir parameter is required."
    show_help
    exit 1
fi

# 检查目录是否存在
if [[ ! -d "$NETFLOW_INSTALL_DIR" ]]; then
    echo "Directory $NETFLOW_INSTALL_DIR does not exist."
    exit 1
fi

# 生成 NetFlowExecmd.service 内容
SERVICE_CONTENT="[Unit]
Description=TinyNetFlow execmd Application Service
After=network.target

[Service]
# install dir
WorkingDirectory=$NETFLOW_INSTALL_DIR
ExecStart=sudo ./daemon/execmd -a ./apps
Restart=on-failure
User=nobody
Group=nogroup
TimeoutStartSec=10
TimeoutStopSec=10

[Install]
WantedBy=multi-user.target"

# 将内容写入 NetFlowExecmd.service 文件
echo "$SERVICE_CONTENT" | sudo tee /etc/systemd/system/NetFlowExecmd.service > /dev/null

# 刷新 systemd 配置
sudo systemctl daemon-reload
# 跟随系统启动
sudo systemctl enable clash

# 输出 success
echo "success install NetFlowExecmd.service in /etc/systemd/system/"
