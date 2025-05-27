当前是在 ubuntu20.04上用自己编译的protobuf生成的。
不同版本生成的代码会有部分差异。

```bash
fzy@fzy-Lenovo:~/$ protoc --version
libprotoc 28.0-dev
```

代码生成
```bash
protoc --cpp_out=./ --proto_path=./ *.proto
```