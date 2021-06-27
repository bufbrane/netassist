# NetAssist网络调试助手

本项目是对Windows平台软件[NetAssist网络调试助手](http://www.cmsoft.cn/resource/102.html)的模仿，利用[libuv](https://github.com/libuv/libuv)库实现从控制台异步收发TCP/UDP数据。

## 使用方法

`-t`: 指定TCP套接字；`-u`: 指定UDP套接字

> 注意：`-t`和`-u`必选且只能选择一个

`-b [address] <port>`: bind，绑定本地IP地址和端口号

> 注意：服务器必须且至少绑定监听的端口号，客户端为可选选项。

`-l`: listen，监听端口号

> 注意：仅用于TCP服务器，UDP无需指定。

`-c <address> <port>`: connect，连接远端IP地址和端口号

> 注意：（高情商）为了保持简洁性，非核心功能如域名解析等暂不支持。（低情商）我懒得做域名解析功能，你自己不会nslookup一下嘛？

`-h`: 打印帮助信息

## 示例

1. 模拟telnet连接[中国科大瀚海星云BBS](http://bbs.ustc.edu.cn)（其telnet服务使用的是GB2312编码，在UTF-8环境下会乱码）：
    ```bash
    ./netassist -tc 202.38.64.3 23
    ```

2. 模拟scanf-printf程序（从控制台读取数据并通过UDP发送套接字向127.0.0.1:8888发送数据；UDP接收套接字监听127.0.0.1:8888，将接收到的数据打印到控制台上）  
    ```bash
    ./netassist -ub 127.0.0.1 8888 -c 127.0.0.1 8888
    ```

3. 客户端向服务器发送文件（利用shell的管道符和重定向实现，打开两个终端并分别执行如下命令）  
    ```bash
    # 服务器端做好接收准备：
    ./netassist -tlb 127.0.0.1 8888 > received_file
    # 客户端向其发送文件：
    cat send_file | ./netassist -tc 127.0.0.1 8888
    ```

