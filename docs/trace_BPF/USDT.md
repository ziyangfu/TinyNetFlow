检查Linux发行版ubuntu， 如果不是，则是通用嵌入式平台，手动编译/事先编译好的 systemtap-sdt
嵌入式Linux平台
   https://sourceware.org/systemtap/wiki
    https://github.com/jav/systemtap
学习systemtap  https://github.com/lichuang/awesome-systemtap-cn

USDT追踪需要在 SEC("path_to_app:xx:xx")或者获得程序运行的PID，这个可以向execmd发出请求，获取程序的pid与源路径

查看应用程序是否已经使能USDT挂载
```shell
sudo bpftrace -lv usdt:/proc/$APP_PID/mem_monitor:*
sudo bpftrace -l 'usdt:<path_to_trace_app>'

fzy@fzy-Lenovo:~/Downloads/04_bcc_ebpf/test$ readelf -n ./app | grep stapsdt
Displaying notes found in: .note.stapsdt
  stapsdt              0x0000004a	NT_STAPSDT (SystemTap probe descriptors)
  stapsdt              0x00000037	NT_STAPSDT (SystemTap probe descriptors)
```


[可观测性 USDT](https://www.ebpflab.com/posts/%E5%8F%AF%E8%A7%82%E6%B5%8B%E6%80%A7usdt/)

