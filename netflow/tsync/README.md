时间同步库
包括一个时间同步库，与一个时间同步守护进程

ROS message_filters 是ROS（机器人操作系统）中的一个功能包，用于实现多个传感器数据或消息的时间同步。

时间同步是意义的

时间同步库是一个用于实现时间同步的库，它提供了多种时间同步算法，如NTP（网络时间协议）、PTP（精确时间协议，Precision Time Protocol）等。

基准时间，所有时间对齐基准时间？

时间同步守护进程是一个用于实现时间同步的守护进程，它负责管理时间同步算法，并确保所有时间同步算法的实现都一致。
为ECU内部和ECU之间的应用提供时间同步。


```bash
$ ethtool -T eth0
Time stamping parameters for eth0:
Capabilities:
	hardware-transmit
	software-transmit
	hardware-receive
	software-receive
	software-system-clock
	hardware-raw-clock
PTP Hardware Clock: 0
Hardware Transmit Timestamp Modes:
	off
	on
Hardware Receive Filter Modes:
	none
	all
	ptpv1-l4-event
	ptpv1-l4-sync
	ptpv1-l4-delay-req
	ptpv2-l4-event
	ptpv2-l4-sync
	ptpv2-l4-delay-req
	ptpv2-event
	ptpv2-sync
	ptpv2-delay-req
```

