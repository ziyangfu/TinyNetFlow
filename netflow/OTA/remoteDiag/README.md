车辆远程诊断，基本思路为：车内端使用DoIP协议，车云之间使用MQTT与HTTPS协议
TSP发送诊断报文，作为payload包裹在HTTPS协议中，进入车端然后解包，发送到
目标控制器，进行诊断

存在形式： remoteDiagDaemon 守护进程