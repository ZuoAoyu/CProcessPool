#include "head.h"

int makeChild(processData_t *pProcessData, int processNum) {
  pid_t pid;

  for (int i = 0; i < processNum; ++i) {
    int pipeFd[2];
    socketpair(AF_LOCAL, SOCK_STREAM, 0, pipeFd);

    // 约定 父进程 使用套接字 sv[0]
    // 子进程 使用套接字 sv[1]

    pid = fork();
    if (pid == 0) {
      // 子进程
      close(pipeFd[0]); // 关闭父进程使用的套接字
      handleEvent(pipeFd[1]);
    } else {
      // 父进程
      close(pipeFd[1]); // 关闭子进程使用的套接字

      printf("new worker pid = %d, pipefd[0] = %d\n", pid, pipeFd[0]);

      pProcessData[i].pid = pid;
      pProcessData[i].status = FREE;
      pProcessData[i].pipeFd = pipeFd[0];
    }
  }

  return 0;
}

void handleEvent(int pipeFd) {
  int netFd;
  while (1) {
    recvFd(pipeFd, &netFd);

    char buf[1024] = {0};
    recv(netFd, buf, sizeof(buf), 0);
    printf("server worker: %s\n", buf);

    send(netFd, buf, strlen(buf), 0);

    close(netFd);

    sleep(3); // 用于演示，减缓工作进程的处理速度

    // 通知父进程任务已完成，应重新将自己设为空闲
    pid_t pid = getpid();
    send(pipeFd, &pid, sizeof(pid_t), 0);
  }
}
