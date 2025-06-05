## 用户态探针列表（USDT user probe list）

> netflow提前预置了USDT静态探针，开发者可以通过埋点获取关键运行时信息，进而辅助分析性能瓶颈、调度行为和资源竞争等问题
>
> 轻量、高效、自由

### 1. osadaptor
#### 1.1 osa_net

| 用户态挂载点   | 含义          | 参数 |
| -------------- | ------------- | ---- |
| tcp_connect    | 建立TCP连接   |      |
| tcp_disconnect | 关闭TCP连接   |      |
| tcp_send       | TCP数据发送   |      |
| tcp_recv       | TCP数据接收   |      |
| tcp_close      | 关闭TCP描述符 |      |
| udp_send_to    | UDP数据发送   |      |
| udp_recv_from  | UDP数据接收   |      |
| udp_close      | 关闭UDP描述符 |      |
| udp_multicast  | UDP组播相关   |      |

#### 1.2 osa_IPC

| 用户态挂载点        | 含义                           | 参数  |
| ------------------- | ------------------------------ | ----- |
| uds_send_to         | UDS数据发送                    | data  |
| uds_recv_from       | UDS数据接收                    | data  |
| uds_send_with_memfd | 通过UDS发送memfd，共享内存同步 | memfd |
| uds_recv_with_memfd | 通过UDS接收memfd，共享内存同步 | memfd |
| shm_write           | 共享内存数据写入ringbuffer     | data  |
| shm_read            | 共享内存从ringbuffer中读数据   | data  |
| shm_create          | 创建共享内存                   |       |
| shm_map             | 创建共享内存映射               |       |
| shm_unmap           | 取消共享内存映射               |       |

#### 1.3 osa_process

| 用户态挂载点      | 含义                          | 参数                    |
| ----------------- | ----------------------------- | ----------------------- |
| process_create    | 进程创建                      | processPid_ , settings_ |
| cgroup_create     | cgroup创建                    | currentCgroupPath_      |
| isolate_cpu_set   | 设置CPU核心隔离（实时性）     | std::vector<int> cpus   |
| isolate_cpu_unset | 取消设置CPU核心隔离（实时性） | 0                       |

#### 1.4 osa_thread

| 用户态挂载点   | 含义                                    | 参数                       |
| -------------- | --------------------------------------- | -------------------------- |
| pthread_create | pthread的线程创建，线程栈及亲和性等设置 | threadHandler_ , settings_ |
|                |                                         |                            |

#### 1.5 osa_coroutine

| 用户态挂载点      | 含义                                                         | 参数             |
| ----------------- | ------------------------------------------------------------ | ---------------- |
| context_swap      | 协程上下文的切换（记录协程之间的切换行为，用于分析调度效率、抢占式/协作式调度策略） | old_ucp, new_ucp |
| coroutine_create  | 协程的创建                                                   |                  |
| coroutine_destroy | 协程的销毁                                                   |                  |
| coroutine_wait    | 协程等待与唤醒（分析协程阻塞原因（如 I/O、锁、信号量等），优化调度逻辑） |                  |



六、总结
类别
探针名称
说明
生命周期
coroutine__create, coroutine__destroy
监控协程生命周期
上下文切换
coroutine__context__switch
分析调度效率
等待事件
coroutine__wait__start, coroutine__wait__end
观察阻塞原因
调度器行为
scheduler__idle, scheduler__enqueue
分析调度器负载
I/O 行为
io__read__start, io__read__done
追踪 I/O 效率
同步机制
sync__mutex__lock, sync__mutex__unlock
分析锁竞争
事件循环
eventloop__tick__start, eventloop__tick__end
监控事件处理延迟



#### 1.6 osa_reactor

| 用户态挂载点 | 含义 | 参数 |
| ------------ | ---- | ---- |
| c            |      | o    |
|              |      |      |
|              |      |      |



### 2. execManagerClient


### 3. execmd


### 4. someipd


### 5. com