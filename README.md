# 描述

1. 不同级别的日志打印
2. 线程池维护运行
3. 异步 tcp 收发
4. 使用自定义传输协议

# 项目构建

1. protobuf 的安装。

```bash
wget  https://github.com/protocolbuffers/protobuf/releases/download/v3.19.4/protobuf-cpp-3.19.4.tar.gz

tar -xzvf protobuf-cpp-3.19.4.tar.gz

cd protobuf-3.19.4

./configure -prefix=/usr/local

make -j4

sudo make install
```

> [!NOTE]
> 卸载使用：`sudo make uninstall`

2. protobuf 的文件和生成文件位于 src/Proto 文件夹下

> [!NOTE]
> 若要生成文件，需要在 proto 文件添加：`option cc_generic_services = true;`，再使用命令（例子）：`protoc --cpp_out=./ ./src/Proto/hello.proto`

# 效果

单连接，单工作线程下，每秒请求达 150k+
