/**
 * 进程池
 */
#include "head.h"

// 保存匿名管道两端的描述符，
// 索引 0 是读端的文件描述符，索引 1 是写端的文件描述符
int pipeFd[2];

void sigFunc(int signum) {
  printf("signum = %d\n", signum);

  // 向管道写端写入数据
  write(pipeFd[1], "1", 1);
  // 发送什么无所谓，只要发过去了，管道读端就绪了，就知道程序该退出了
  // ssize_t write(int fd, const void *buf, size_t count);
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "args failed.\n");
    fprintf(stderr, "usage: ./server 127.0.0.1 2338 10");
    exit(-1);
  }

  // 创建匿名管道
  pipe(pipeFd);

  // 工作进程的数量
  int workerNum = atoi(argv[3]);

  // workerList 访问所有工作进程的状态 改为全局变量
  processData_t *workerList =
      (processData_t *)calloc(sizeof(processData_t), workerNum);

  // 注册信号处理函数
  signal(SIGUSR1, sigFunc);

  // 创建指定数量的子进程作为工作进程
  makeChild(workerList, workerNum);

  // 初始化 tcp 连接
  int sockFd;
  tcpInit(argv[1], argv[2], &sockFd);

  // 初始化epoll
  int epfd = epollCreate();
  epollAdd(sockFd, epfd);
  for (int i = 0; i < workerNum; ++i) {
    epollAdd(workerList[i].pipeFd, epfd);
  }

  // 将 管道读端 添加到 epoll 实例中进行监控
  epollAdd(pipeFd[0], epfd);

  int listenSize = workerNum + 1; // 监听的 fd 的数量
  // （1 个 socket 和 每个进程的 本地套接字 的一端（只用来读））

  struct epoll_event *readyList =
      (struct epoll_event *)calloc(listenSize, sizeof(struct epoll_event));

  while (1) {
    int readyNum = epoll_wait(epfd, readyList, listenSize, -1);
    for (int i = 0; i < readyNum; ++i) {
      if (readyList[i].data.fd == sockFd) {
        puts("accept ready");

        int netFd = accept(sockFd, NULL, NULL);
        for (int j = 0; j < workerNum; ++j) {
          if (workerList[j].status == FREE) {
            printf("No. %d worker gets his job, pid = %d\n", j,
                   workerList[j].pid);
            sendFd(workerList[j].pipeFd, netFd);
            workerList[j].status = BUSY;
            break;
          }
        }

        close(netFd); // 父进程把 netFd 的处理交给了子进程，所以父进程关闭套接字
      } else if (readyList[i].data.fd == pipeFd[0]) {
        for (int i = 0; i < workerNum; ++i) {
          printf("send signal to worker %d.\n", workerList[i].pid);
          kill(workerList[i].pid, SIGKILL);
        }

        for (int i = 0; i < workerNum; ++i) {
          wait(NULL);
        }

        puts("process pool is over!");
        exit(0);
      } else {
        puts("one worker finish its task");

        for (int j = 0; j < workerNum; ++j) {
          if (workerList[j].pipeFd == readyList[i].data.fd) {
            pid_t pid;
            int ret = recv(workerList[j].pipeFd, &pid, sizeof(pid_t), 0);

            printf("No. %d worker finish, pid = %d\n", j, pid);

            workerList[j].status = FREE;
            break;
          }
        }
      }
    }
  }
}
