#ifndef __HEAD_H
#define __HEAD_H
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define ERROR_CHECK(result, num, msg)                                          \
  {                                                                            \
    if (result == num) {                                                       \
      perror(msg);                                                             \
      return -1;                                                               \
    }                                                                          \
  }

enum workerStatus { FREE, BUSY };

typedef struct {
  pid_t pid;  // pid of working process
  int status; // status of working process
  int pipeFd; // 本地套接字，与父进程通信
} processData_t;

void handleEvent(int pipeFd);
int makeChild(processData_t *pProcessData, int processNum);

int tcpInit(char *ip, char *port, int *pSockFd);

// 发送和接收 文件对象 的访问权
int sendFd(int pipeFd, int fdToSend);
int recvFd(int pipeFd, int *pfdtorecv);

int epollCreate();
int epollAdd(int fd, int epfd);
int epollDel(int fd, int epfd);

typedef struct train_s {
  int dataLength;
  char buf[1000];
} train_t;

int recvFile(int netFd);
int sendFile(int netFd);

int recvn(int netFd, void *pstart, int len);

#endif
