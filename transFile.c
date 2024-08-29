#include "head.h"

int recvFile(int netFd) {
  train_t t;

  // 接收 文件名长度 和 文件名
  recv(netFd, &t.dataLength, sizeof(int), 0);
  recv(netFd, t.buf, t.dataLength, 0);

  t.buf[t.dataLength] = '\0';

  // 接收文件内容
  // 先创建文件
  char filepath[100] = "temp/";
  strcat(filepath, t.buf);
  int fd = open(filepath, O_WRONLY | O_CREAT, 0666);
  ERROR_CHECK(fd, -1, "open");

  // 接收 文件内容长度 和 文件内容
  bzero(&t, sizeof(t));
  int ret = recv(netFd, &t.dataLength, sizeof(int), 0);
  ERROR_CHECK(ret, -1, "recv");
  ret = recv(netFd, t.buf, t.dataLength, 0);
  ERROR_CHECK(ret, -1, "recv");

  write(fd, t.buf, t.dataLength);

  // 关闭打开的文件的 文件描述符
  close(fd);
  return 0;
}

int sendFile(int netFd) {
  train_t t = {5, "file1"};

  // 发送 文件名长度 和 文件名
  send(netFd, &t.dataLength, sizeof(int), 0);
  send(netFd, t.buf, t.dataLength, 0);

  // 打开文件
  int fd = open(t.buf, O_RDONLY);
  ERROR_CHECK(fd, -1, "open");

  // 读取文件内容
  bzero(&t, sizeof(t));
  int ret = read(fd, t.buf, sizeof(t.buf));
  t.dataLength = ret;

  // 发送 文件内容长度 和 文件内容
  send(netFd, &t.dataLength, sizeof(int), 0);
  send(netFd, t.buf, t.dataLength, 0);

  return 0;
}
