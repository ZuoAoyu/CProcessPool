#include "head.h"

int main() {
  int sv[2];

  int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
  if (ret == -1) {
    perror("socketpair");
    exit(-1);
  }

  printf("socketpair, sv[0] = %d, sv[1] = %d\n", sv[0], sv[1]);

  printf("pid = %d\n", getpid());
  sleep(5); // 为运行命令 `lsof -p {pid}` 留出时间

  // 约定 父进程 使用套接字 sv[0]
  // 子进程 使用套接字 sv[1]

  if (fork() == 0) {
    // 子进程
    close(sv[0]); // 关闭父进程的套接字

    int fdfile;
    // 从另一端（父进程）接收 文件对象 的"访问权"
    recvFd(sv[1], &fdfile);
    printf("child fdfile = %d\n", fdfile);

    // 写入数据到文件
    write(fdfile, "nice to meet you", 16);

    close(sv[1]); // 关闭子进程的套接字

  } else {
    close(sv[1]); // 关闭子进程的套接字

    int fdfile = open("file1", O_RDWR);
    printf("parent fdfile = %d\n", fdfile);

    // 写入数据到文件
    write(fdfile, "hello world", 11);

    // 发送 文件对象 的"访问权" 到另一端（子进程）
    sendFd(sv[0], fdfile);

    close(sv[0]); // 关闭父进程的套接字
    wait(NULL);   // 等待子进程结束
  }

  return 0;
}

/*
运行流程：

编译程序：gcc test_shareFd.c shareFd.c
在与程序同一目录下创建文件 "file1"
运行程序：./a.out

output:
$ ./a.out
socketpair, sv[0] = 3, sv[1] = 4
pid = 1795533
parent fdfile = 4
buf = hello, fd = 3
child fdfile = 3

程序运行完毕后，查看文件 file1，在里面可以看到从父进程和子进程写入的内容：
hello worldnice to meet you

这也说明了，虽然在父进程和子进程中，被传递的文件对象的文件描述符不同，但的确是同一个文件对象。它们的读写偏移量也是共享的。
和之前 dup 的例子，以及 先打开文件再 fork 的例子中一样。
*/

/*
查看进程中打开的文件描述符(fd, file descriptor)
命令：lsof -p {pid}
在上面代码中取消注释 sleep()，在代码运行到那里暂停时，输入命令 lsof -p
{pid}，可观察到 本地套接字 的两端分别使用了文件描述符 3 和 4。
输出如：

# lsof -p 1795627
COMMAND     PID USER   FD   TYPE             DEVICE SIZE/OFF     NODE NAME
a.out   1795627 root  cwd    DIR              259,1     4096  1585210 /root/test
a.out   1795627 root  rtd    DIR              259,1     4096        2 /
a.out   1795627 root  txt    REG              259,1    17408  1585550 /root/test/a.out
a.out   1795627 root  mem    REG              259,1  1901536 41684390 /usr/lib/x86_64-linux-gnu/libc-2.31.so
a.out   1795627 root  mem    REG              259,1   177928 41684386 /usr/lib/x86_64-linux-gnu/ld-2.31.so
a.out   1795627 root    0u   CHR              136,1      0t0        4 /dev/pts/1
a.out   1795627 root    1u   CHR              136,1      0t0        4 /dev/pts/1
a.out   1795627 root    2u   CHR              136,1      0t0        4 /dev/pts/1
a.out   1795627 root    3u  unix 0x00000000c795d4f7      0t0 56845562 type=STREAM
a.out   1795627 root    4u  unix 0x0000000080ddac00      0t0 56845563 type=STREAM
*/
