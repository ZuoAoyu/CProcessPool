#include "head.h"

off_t getFileSize(int fd) {
  // 记录当前文件指针的位置
  off_t currentPos = lseek(fd, 0, SEEK_CUR);
  if (currentPos == (off_t)-1) {
    perror("lseek");
    return -1;
  }

  // 将文件指针移动到文件末尾；
  // 返回值是文件开头到当前指针位置的文件偏移量，
  // 也即文件大小
  off_t fileSize = lseek(fd, 0, SEEK_END);
  if (fileSize == (off_t)-1) {
    perror("lseek");
    return -1;
  }

  // 将文件指针恢复到之前的位置
  if (lseek(fd, currentPos, SEEK_SET) == (off_t)-1) {
    perror("lseek");
    return -1;
  }

  return fileSize;
}

int recvn(int netFd, void *buf, int total) {
  char *p = (char *)buf;
  long cursize = 0;
  while (cursize < total) {
    ssize_t sret = recv(netFd, p + cursize, total - cursize, 0);
    if (sret == 0) {
      return 1;
    }
    cursize += sret;
  }

  return 0;
}

int recvFile(int netFd) {
  train_t t;
  bzero(&t, sizeof(t));

  // 接收 文件名长度 和 文件名
  recv(netFd, &t.dataLength, sizeof(int), 0);
  recv(netFd, t.buf, t.dataLength, 0);

  t.buf[t.dataLength] = '\0';

  // 接收文件内容
  // 先创建文件
  char filepath[100] = "temp/";
  strcat(filepath, t.buf);
  int fd = open(filepath, O_RDWR | O_CREAT, 0666); // 由 O_WRONLY 改为 O_RDWR
  ERROR_CHECK(fd, -1, "open");

  // 接收文件总大小
  off_t fileSize;
  bzero(&t, sizeof(t));
  recvn(netFd, &t.dataLength, sizeof(int));
  recvn(netFd, &fileSize, t.dataLength);
  printf("fileSize = %ld\n", fileSize);

  // 调整文件大小到指定大小
  ftruncate(fd, fileSize);

  // 建立磁盘文件和用户态内存之间的映射
  // 因为后面需要读写 mmap 映射的数据，所以前面 open 时的权限改为 O_RDWR
  char *p =
      (char *)mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  ERROR_CHECK(p, MAP_FAILED, "mmap");

  // 接收文件内容，一次性接收完
  recvn(netFd, p, fileSize);

  // 解除映射
  munmap(p, fileSize);

  // 关闭打开的文件的 文件描述符
  close(fd);
  return 0;
}

int sendFile(int netFd) {
  train_t t = {5, "file2"};

  // 发送 文件名长度 和 文件名
  send(netFd, &t, sizeof(int) + t.dataLength, 0);

  // 打开文件
  int fd = open(t.buf, O_RDONLY);
  ERROR_CHECK(fd, -1, "open");

  // 文件状态信息
  struct stat statbuf;
  int ret = fstat(fd, &statbuf);

  // 发送文件大小
  bzero(&t, sizeof(t));
  t.dataLength = sizeof(statbuf.st_size);
  memcpy(t.buf, &statbuf.st_size, t.dataLength);
  send(netFd, &t, sizeof(int) + sizeof(off_t), MSG_NOSIGNAL);

  // 建立磁盘文件和用户态内存之间的映射
  char *p = (char *)mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
  ERROR_CHECK(p, MAP_FAILED, "mmap");

  off_t total = 0;
  while (total < statbuf.st_size) {
    bzero(&t, sizeof(t));

    // “小火车”每次运多少数据
    if (statbuf.st_size - total > sizeof(t.buf)) {
      t.dataLength = sizeof(t.buf);
    } else {
      t.dataLength = statbuf.st_size - total;
    }

    memcpy(t.buf, p + total, t.dataLength);
    total += t.dataLength;

    // 发送数据
    // 注意只发送缓冲区，因客户端是一次性接收
    ret = send(netFd, &t.buf, t.dataLength, MSG_NOSIGNAL);
    if (ret == -1) {
      perror("send");
      break;
    }
  }

  // 结束的时候发送一个车厢为 0 的小火车（结束标志）
  t.dataLength = 0;
  send(netFd, &t, sizeof(int), MSG_NOSIGNAL);

  munmap(p, statbuf.st_size);
  close(fd);
  return 0;
}
