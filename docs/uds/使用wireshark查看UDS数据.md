## wireshark查看UDS数据

目前还没搞清楚socat的使用方法，无法捕捉到数据。

```bash
sudo socat -t100 -d -x -v TCP-LISTEN:65230,reuseaddr,fork UNIX-CONNECT:/tmp/uds.socket 
```

```c++
// uds_client.cpp
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

const char* SOCKET_PATH = "/tmp/uds.socket";

int main() {
    // 创建 Unix Domain Socket
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // 连接到服务器
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cerr << "Failed to connect to server\n";
        close(sock_fd);
        return 1;
    }

    std::cout << "Connected to server\n";

    // 发送数据
    const char* message = "Hello from client!";
    write(sock_fd, message, strlen(message));

    // 接收响应
    char buffer[128];
    ssize_t bytes_read = read(sock_fd, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        std::cout << "Received: " << buffer << "\n";
    }

    // 关闭连接
    close(sock_fd);

    return 0;
}
```

```c++
// uds_server.cpp
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

const char* SOCKET_PATH = "/tmp/uds.socket";

int main() {
    // 创建 Unix Domain Socket
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // 绑定到文件路径
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // 删除已存在的 socket 文件
    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cerr << "Failed to bind socket\n";
        close(server_fd);
        return 1;
    }

    // 监听连接
    if (listen(server_fd, 5) == -1) {
        std::cerr << "Failed to listen on socket\n";
        close(server_fd);
        return 1;
    }

    std::cout << "Server is listening on " << SOCKET_PATH << "\n";

    // 接受客户端连接
    int client_fd;
    struct sockaddr_un client_addr;
    socklen_t client_len = sizeof(client_addr);

    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd == -1) {
        std::cerr << "Failed to accept connection\n";
        close(server_fd);
        return 1;
    }

    std::cout << "Client connected\n";

    // 接收数据
    char buffer[128];
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        std::cout << "Received: " << buffer << "\n";
    }

    // 发送响应
    const char* response = "Hello from server!";
    write(client_fd, response, strlen(response));

    // 关闭连接
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}
```

```c++
// tcp_client.cpp
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <chrono>

int main() {
    // 创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return 1;
    }

    // 设置服务器地址
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(65230);  // 端口号为65239
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    // 发送数据
    const char* message = "hello world";
    while (true) {
        // 发送数据
        ssize_t bytes_sent = send(sockfd, message, strlen(message), 0);
        if (bytes_sent == -1) {
            perror("send");
            break;
        }

        // 打印发送的数据
        std::cout << "Sent: " << message << std::endl;

        // 等待1秒
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // 关闭套接字
    close(sockfd);

    return 0;
}

```

### socat转发uds数据至TCP端口

### 其他方式
- bpftrace
```shell
#!/usr/bin/env bpftrace
// trace_send.bt
// 追踪特定进程的 sys_enter_sendto 和 sys_enter_recvfrom
//tracepoint:syscalls:sys_enter_recvfrom
tracepoint:syscalls:sys_enter_sendto
/pid == 56908/  // 仅追踪 PID 为 65578 的进程
{
    // 打印进程名、PID、系统调用名、文件描述符、缓冲区内容和长度
    printf(
        "PID: %d, Comm: %s, Syscall: %s, FD: %d, Addr: 0x%x, Len: %d",
        pid, comm, probe, args->fd, args->buff, args->len
    );
}
```
- strace
```bash
# -p pid
sudo strace -p 60533 -tt -f -y -s 65535 -e read write 
```

### 参考文档
[通过tcpdump对Unix Domain Socket 进行抓包解析](https://plantegg.github.io/2018/01/01/%E9%80%9A%E8%BF%87tcpdump%E5%AF%B9Unix%20Socket%20%E8%BF%9B%E8%A1%8C%E6%8A%93%E5%8C%85%E8%A7%A3%E6%9E%90/)
[socat 实用网路工具和使用解析unix socket 方法](https://malagege.github.io/blog/posts/socat-%E5%AF%A6%E7%94%A8%E7%B6%B2%E8%B7%AF%E5%B7%A5%E5%85%B7-%E5%92%8C-%E4%BD%BF%E7%94%A8%E8%A7%A3%E6%9E%90-unix-socket-%E6%96%B9%E6%B3%95/)