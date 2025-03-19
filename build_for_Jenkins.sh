#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# \brief Build script for Jenkins. must running in root permission
# \file build_for_Jenkins.sh
# \usage
      # 调试模式，仅编译安装，跳过依赖检查并跳过测试
          # ./build_for_Jenkins.sh
      # 测试模式，跳过依赖检查，执行编译安装与单元测试
          # ./build_for_Jenkins.sh -t
      # 执行所有步骤， 执行依赖检查、编译安装与单元测试
          # ./build_for_Jenkins.sh -n -t
# -----------------------------------------------------------------------------
# start
# 参数处理
RUN_CHECKS=0
RUN_TESTS=0
show_help() {
    echo "Usage: $0 [-n] [-t] [-h]"
    echo "Build script for Jenkins. Only compile and install in default mode"
    echo "Options:"
    echo "  -n    run dependency checks"
    echo "  -t    run unit tests execution"
    echo "  -h    Show this help message"
    exit 0
}
while getopts "nth" opt; do
    case $opt in
        n) RUN_CHECKS=1 ;;
        t) RUN_TESTS=1 ;;
        h) show_help ;;
        *) echo "Invalid option: -$OPTARG" >&2; exit 1 ;;
    esac
done

# step 0: check root permission
if [ "$(id -u)" != "0" ]; then
    echo "This script must be run as root."
    exit 1
fi

# step 1: build dependencies
# 原来的思路：
# check third lib like spdlog, protobuf, googletest, systemtap ... is installed
# if not installed
    # check <project>/third_party/<third_party> is empty?
        # if empty
            # use git submodule init and update it
        # if not empty,
            # build and install it
if [ $RUN_CHECKS -eq 1 ]; then
    echo "===== Checking Build Dependencies ====="
    # 基础工具检查
    declare -a required_commands=(cmake git pkg-config)
    for cmd in "${required_commands[@]}"; do
        if ! command -v $cmd &> /dev/null; then
            echo "Installing $cmd..."
            apt-get install -y $cmd || { echo "Failed to install $cmd"; exit 1; }
        fi
    done
    # USDT 安装
    apt install systemtap-sdt-dev
    # 第三方库检查
    declare -A required_libs=(
        ["spdlog"]="libspdlog-dev"
        ["protobuf"]="libprotobuf-dev protobuf-compiler"
        ["gtest"]="libgtest-dev"
    )
    for lib in "${!required_libs[@]}"; do
        if ! pkg-config --exists $lib; then
            echo "Installing ${required_libs[$lib]}..."
            apt-get install -y ${required_libs[$lib]} || { echo "Failed to install $lib"; exit 1; }
        fi
    done
    # 子模块初始化
    if [ -z "$(ls -A ./third_party)" ]; then
        echo "Initializing git submodules..."
        git submodule init && git submodule update || { echo "Submodule init failed"; exit 1; }
    fi
else
    echo "===== Skipping Dependency Checks ====="
fi

# step 2: build netflow
echo "===== Building TinyNetFlow ====="
# rm -rf ./build
# mkdir -p build || exit 1
cd build || exit 1
#cmake -DCMAKE_INSTALL_PREFIX=./install \
#      -DCMAKE_BUILD_TYPE=Release \
#      -DBUILD_TESTING=ON .. || { echo "CMake failed"; exit 1; }

cmake -DCMAKE_INSTALL_PREFIX=./install .. || { echo "CMake failed"; exit 1; }
make -j$(nproc) || { echo "Build failed"; exit 1; }
make install || { echo "Install failed"; exit 1; }

# step 3: 条件执行单元测试
if [ $RUN_TESTS -eq 1 ]; then
    echo "===== Running Unit Tests ====="
    cd ./install/tests || exit 1
    ./run_all_tests.sh -e || { echo "Tests failed"; exit 1; }
else
    echo "===== Skipping Unit Tests ====="
fi
echo "===== Build Successful ====="
# end