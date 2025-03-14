#!/bin/bash
# 脚本：run_all_tests.sh
# 功能：运行所有单元测试程序
# 作者：fzy
# 创建时间：2025
# 使用范例：
# sudo ./run_all_tests.sh [opt]

# 函数：显示帮助信息
show_help() {
    echo "Usage: $0  [-h] [-l] [-e]"
    echo "running unit test all in one, this script must be run as root"
    echo "  -h                  Show this help message."
    echo "  -l                  List all executable files in the current directory."
    echo "  -e                  Execute all executable files in the current directory with superuser privileges."
    exit 0
}

# 初始化变量
list_executables=false
execute_executables=false

# 解析命令行参数
while getopts "hle" opt; do
    case $opt in
        h)
            show_help
            ;;
        l)
            list_executables=true
            ;;
        e)
            execute_executables=true
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            show_help
            ;;
    esac
done

# 显示可执行文件
if $list_executables; then
    echo "Listing all executable files in the current directory:"
    find . -maxdepth 2 -type f -executable -not -name "run_all_tests.sh" -print
fi

# 执行可执行文件
if $execute_executables; then
    # 检查是否具有root权限
    if [[ $EUID -ne 0 ]]; then
        echo "This script must be run as root. Please use sudo if in ubuntu/debian."
        exit 1
    fi
    echo "Executing all executable files in the current directory with superuser privileges:"
    find . -maxdepth 2 -type f -executable -not -name "run_all_tests.sh" | while read -r file; do
        echo "Executing $file"
        sudo "$file"
    done
fi

# 如果没有提供任何参数，显示帮助信息
if ! $list_executables && ! $execute_executables; then
    show_help
fi
