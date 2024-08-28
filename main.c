/**
 * 一个最基本的进程池：
 * 客户端读取标准输入发送到服务端，
 * 服务端主进程收到连接请求后将任务交给一个空闲的工作进程，
 * 处理请求的工作进程回复一个相同内容的字符串给客户端。
./server 127.0.0.1 2338 10
 */
#include "head.h"

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "args failed.\n");
    fprintf(stderr, "usage: ./server 127.0.0.1 2338 10");
    exit(-1);
  }

  // 工作进程的数量
  int workerNum = atoi(argv[3]);

  // workerList 访问所有工作进程的状态
  processData_t *workerList =
      (processData_t *)calloc(sizeof(processData_t), workerNum);

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
