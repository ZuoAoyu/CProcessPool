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

  // 创建一个匿名管道
  int pipefds[2];
  pipe(pipefds);

  int total = 0;
  while (total < fileSize) {
    int ret = splice(netFd, NULL, pipefds[1], NULL, 4096, SPLICE_F_MORE);
    total += ret;
    splice(pipefds[0], NULL, fd, NULL, ret, SPLICE_F_MORE);
  }

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

  // 使用 sendfile 发送文件
  sendfile(netFd, fd, NULL, statbuf.st_size);

  // // 结束的时候发送一个车厢为 0 的小火车（结束标志）
  // t.dataLength = 0;
  // send(netFd, &t, sizeof(int), MSG_NOSIGNAL);

  close(fd);
  return 0;
}

/*
#define _GNU_SOURCE         // See feature_test_macros(7)
#include <fcntl.h>

ssize_t splice(int fd_in, loff_t *off_in, int fd_out,
              loff_t *off_out, size_t len, unsigned int flags);

fd_in：源文件描述符，从这个文件描述符中读取数据。
off_in：指向源文件偏移量的指针。如果 off_in 为
NULL，则使用文件描述符当前的文件偏移量，并自动更新偏移量。否则，它指向的偏移量将被更新（仅对文件支持）。
fd_out：目标文件描述符，数据将写入到这个文件描述符中。
off_out：指向目标文件偏移量的指针。和 off_in 类似，如果 off_out 为
NULL，则使用文件描述符当前的文件偏移量，并自动更新。 len：要移动的字节数。
flags：控制行为的标志，常用的有：
  SPLICE_F_MOVE：提示内核尽量移动数据，而不是复制数据。实际上，这个标志很少会被实际使用。
  SPLICE_F_NONBLOCK：如果设置，splice 将以非阻塞方式执行。
  SPLICE_F_MORE：提示内核在接下来的操作中可能会有更多的数据要写入，可以帮助优化。
  SPLICE_F_GIFT：表示传输的文件描述符是一个“礼物”，接收方可以将其视为自己所有，类似于文件描述符转移。
*/

/*
在接收方使用 splice 时，发送方在发送结束后不能再发送 结束标志：

```
// 结束的时候发送一个车厢为 0 的小火车（结束标志）
t.dataLength = 0;
send(netFd, &t, sizeof(int), MSG_NOSIGNAL);
```

因为在 recvFile(int netFd)
最后一次循环里，这条数据有可能会被和末尾数据一起读到文件中，导致 接收到的数据 和
发送的数据 不一致。

*/
