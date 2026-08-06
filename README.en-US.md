

# TinyNetFlow
### 1. Brief Introduction

A lightweight open-source communication middleware for the automotive domain. It employs an event-driven architecture and I/O multiplexing mechanisms at its core. Key features include:
- C++17 standard
- Reactor network model
- Not cross-platform; Linux only
- I/O multiplexing uses Linux epoll exclusively
- IPv4 && IPv6
- TCP server/client
- Supports 2 common TCP packet framing methods (delimiter and header length field)
- [TCP supports heartbeat, reconnection, and forwarding]
- UDP server/client
- Supports UDP multicast
- IPC supports Unix domain sockets && shared memory
- Supports protocols commonly used in Connected & Intelligent Vehicles: MQTT, HTTP, and in-vehicle SOME/IP

**Code Quality: Toy Level** | Even salted fish need dreams. Who didn't start at the toy level?
> Toy Level, Usable Level, Industrial Level, Automotive Grade......

### 2. Layered Architecture

<img src="./docs/images/架构图_1.png" style="zoom:25%;" />

																									*Architecture Diagram - Phase I*

<img src="./docs/images/架构图_2.png" style="zoom:22%;" />

																								*Architecture Diagram - Long-term Vision* (Let's dream big first......)

![通信示意图1](./docs/images/通信示意图_1.png)
*Communication Diagram*

### 3. Development Purpose

Fully unleashing the C++ programmer spirit of reinventing the wheel. Haha.

- Understand the Linux kernel network protocol stack -> Networking API -> Network Communication Middleware -> Application, connecting the entire pipeline to achieve full functionality implementation
- The automotive domain has its unique characteristics: highly security-sensitive (functional security, information security) and real-time-sensitive (deterministic communication and scheduling). NetFlow aims to contribute to these areas by utilizing eBPF technology in both the Linux kernel and user space to enable full-stack data observation, diagnostics, and optimization from kernel to user space. Joint development and optimization of the Linux kernel and middleware.

### 4. Build
Dependencies
```bash
# USDT/uprobe dependency
# Tracing functionality is disabled by default; install if needed
sudo apt install systemtap-sdt-dev
```

CMake Build Method
```bash
# Download
git clone git@github.com:ziyangfu/TinyNetFlow.git
git submodule update --recursive
cd <TinyNetFlow_dir>
# Build under the current directory, installation will be placed in build/install
sudo ./build_for_Jenkins.sh -n
```

### 5. Examples
Examples for TCP, UDP & multicast, Unix domain sockets, shared memory, MQTT, 
HTTP, and SOME/IP can be found in the `examples` folder.

Run instructions can be found at the top of each respective `.cpp` file.

### 6. Installation Directory Structure
```bash
.
├── apps
│   ├── <...>             # User applications
├── daemon
│   └── execmd            # Execution management utility
├── etc
│   └── emConfig.json     # Configuration file
├── include
│   ├── execManagerClient # Header files
│   └── osadaptor
│   └── com
└── lib
    ├── cmake              # CMake modules for find_package
    ├── libexecManagerClient.a  # Static library
    └── libosadaptor.a
    └── libcom.a
└── tests
    ├── osadaptor # Unit test suite
    └── run_all_tests.sh   # Run all unit tests
└── trace   # eBPF-related tracing programs

```
### 7. About Tracing
![usdt](./docs/images/usdt.png)
### 8. Acknowledgments
- The underlying networking library was heavily inspired by muduo. Thanks to Chen Shuo.
- MQTT code referenced libhv. Thanks to ithewei.
- SOME/IP referenced vsomeip
