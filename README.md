# 编译 和 运行

```
$ gcc main.c epoll.c shareFd.c tcpInit.c worker.c transFile.c -o server
$ gcc test_client.c transFile.c -o client

$ mkdir temp
$ echo "hello world" > file2

$ ./server 127.0.0.1 2338 10
$ ./client 127.0.0.1 2338
```

运行完程序后，可发现 客户端 将程序目录下的`file2`文件发送到了 服务端，上传到了`temp/`目录下。
