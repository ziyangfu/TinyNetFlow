# POSIX 各个标准的系统调用列表
以下列出各标准的**部分**系统调用。
- Minimal：最小嵌入式子集规范 —— PSE51 
- Controller：工业控制器子集规范 —— PSE52 
- Dedicated：较大规模的嵌入式系统子集规范 —— PSE53 
- Multi-Purpose：具有实时性要求的大规模通用系统子集规范 —— PSE54
## PSE51
snprintf, memset, sigfillset, kill, strtoul, close, pthread_create, strlen, strncpy, strtol, sigprocmask, raise, puts, open, isxdigit, write, memcmp, strrchr, gets, localtime, pthread_detach, strncat, read, sigwait, sigaddset, free, atol, pthread_sigmask, sigaction, memcpy, ctime, isalnum, signal, sigemptyset, strerror, pthread_cancel, time, pthread_self, strcmp, strftime,

## PSE52
fcntl, stat, chdir, log, getcwd, remove, fstat, access, closedir, opendir, link, readdir, unlink, sin, trunc,

## PSE53
getaddrinfo, shutdown, assert, FD_SET, accept, inet_pton, freeaddrinfo, FD_ZERO, connect, getsockname, htons, getnameinfo, select, pipe, getsockopt, ntohl, send, ntohs, recvmsg, getpid, exit, listen, fork, FD_ISSET, gai_strerror, inet_ntop, if_nametoindex, getpeername, recv, wait, setsockopt, socket, bind, recvfrom, sendto, sleep, execv, sendmsg,

## PSE54
getopt, glob, system, truncate,

> 参考：
> 1. https://pubs.opengroup.org/onlinepubs/9799919799/
> 2. [学习笔记：IEEE 1003.13-2003【POSIX PSE54接口列表】](https://blog.csdn.net/qq_350984705/article/details/138765594)