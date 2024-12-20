该文件夹存放UNIX域套接字这种IPC方式的代码


使用uds协议时，采用以下的header，方便接收端拆包

version | length | payload