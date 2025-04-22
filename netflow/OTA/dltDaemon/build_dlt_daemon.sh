#!/usr/bin/env bash
# -----------------------------------------------------------------------------
# \brief Build script for DLT daemon. it will called by top dltDaemon CMakeLists.txt
# \file build_dlt_daemon.sh
# \usage
#      ./build_dlt_daemon.sh [src_dir] [build_dir] [install_dir]
# -----------------------------------------------------------------------------

# Function to display help information
usage() {
    echo "Usage: $0 [src_dir] [build_dir] [install_dir]"
    echo "  src_dir      : Path to the DLT daemon source directory"
    echo "  build_dir    : Path to the build directory"
    echo "  install_dir  : Path to the installation directory"
    echo
    echo "Example:"
    echo "  $0 /path/to/dlt-daemon /path/to/build /path/to/install"
}

# Check if the help flag is provided
if [ "$1" == "-h" ]; then
    usage
    exit 0
fi

# Check if the correct number of arguments is provided
if [ "$#" -ne 3 ]; then
    echo "Error: Invalid number of arguments."
    usage
    exit 1
fi

# Assign command line arguments to variables
src_dir=$1
build_dir=$2
install_dir=$3

# Create build directory if it doesn't exist
mkdir -p "$build_dir"
cd "$build_dir" || exit
cmake -DCMAKE_INSTALL_PREFIX="$install_dir" "$src_dir"
make
make install

echo "DLT Daemon built and installed in temp directory successfully."