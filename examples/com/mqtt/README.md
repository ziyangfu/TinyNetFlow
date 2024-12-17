# MQTT例程

```bash
# 订阅程序
./examples/mqtt/mqttClientSub localhost 1883 test
# 发布程序
./examples/mqtt/mqttClientPub localhost 1883 test apple
# 服务端程序
mosquitto -v
```

订阅方：

![image-20230818144155536](README/image-20230818144155536.png)

![image-20230818144328848](README/image-20230818144328848.png)

发布方：

![image-20230818144405498](README/image-20230818144405498.png)