/*
本地套接字

一对相互连接的套接字

父进程和子进程互相发送数据
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  int sv[2];
  char buf[128] = {0};

  // 创建一对相互连接的套接字
  // sv 是一个长度为 2 的整型数组，
  // 用于存储管道两端的文件描述符
  // （注意，sv[0] 和 sv[1] 没有任何区别）
  int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
  if (ret == -1) {
    perror("socketpair");
    exit(-1);
  }

  // 约定 父进程 使用套接字 sv[0]
  // 子进程 使用套接字 sv[1]

  if (fork() == 0) {
    // 子进程
    close(sv[0]); // 关闭父进程的套接字

    // 从父进程接收消息
    read(sv[1], buf, sizeof(buf));
    printf("子进程收到：%s\n", buf);

    // 给父进程发送消息
    const char *child_msg = "hello from child";
    write(sv[1], child_msg, strlen(child_msg));

    close(sv[1]); // 关闭子进程的套接字

  } else {
    close(sv[1]); // 关闭子进程的套接字

    // 给子进程发送消息
    const char *parent_msg = "hello from parent";
    write(sv[0], parent_msg, strlen(parent_msg));

    // 从子进程接收消息
    read(sv[0], buf, sizeof(buf));
    printf("父进程收到：%s\n", buf);

    close(sv[0]); // 关闭父进程的套接字

    wait(NULL); // 等待子进程结束
  }

  return 0;
}

/*
父进程和子进程的地址空间是隔离的，如果两个进程之间需要进行通信，那就要选择一种合适的进程间通信的手段。除了之前所使用的
pipe 系统调用可以在父子进程间创建管道以外，还有一种方法是本地套接字。

使用系统调用 socketpair 可以在父子进程间利用 socket 创建一个全双工的管道。
除此以外，本地套接字可以在同一个操作系统的两个进程之间传递文件描述符。

创建套接字的方式是和管道的用法十分相似：

int socketpair(int domain, int type, int protocol, int sv[2]);

在这里domain必须填写AF_LOCAL，
type可以选择流式数据还是消息数据，
protocol一般填 0 表示不需要任何额外的协议，
sv 这个参数和 pipe 的参数一样，是一个长度为 2
的整型数组，用来存储管道两端的文件描述符（值得注意的是， sv[0] 和 sv[1]
没有任何的区别）。

一般socketpair之后会配合fork函数一起使用，从而实现父子进程之间的通信。
从数据传递使用上来看，本地套接字和网络套接字是完全一致的，但是本地套接字的效率更高，因为它在拷贝数据的时候不需要处理协议相关内容。

*/
