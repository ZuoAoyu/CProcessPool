#include "head.h"

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
  int fd = open(filepath, O_WRONLY | O_CREAT, 0666);
  ERROR_CHECK(fd, -1, "open");

  int ret;
  while (1) {
    // 接收 文件内容长度 和 文件内容
    bzero(&t, sizeof(t));
    recvn(netFd, &t.dataLength, sizeof(int));
    if (t.dataLength == 0) {
      break;
    }

    /* ret = recv(netFd, t.buf, t.dataLength, 0); */
    /* ERROR_CHECK(ret, -1, "recv"); */
    // 调用recv 时，传入的整型长度参数（这里是t.dataLength）描述的是最大长度，
    // 而不是本次 recv 所能获取到的数据长度。
    // 实际 recv 接收到的数据长度可能无法达到这个长度，
    // 因为TCP是一种流式协议，它只能负责每个报文可靠有序地发送和接收，
    // 但是并不能保证传输到网络缓冲区当中的是完整的一个"小火车"。
    recvn(netFd, t.buf, t.dataLength);

    write(fd, t.buf, t.dataLength);
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

  while (1) {
    // 读取文件内容
    bzero(&t, sizeof(t));
    int ret = read(fd, t.buf, sizeof(t.buf));
    ERROR_CHECK(ret, -1, "read");
    t.dataLength = ret;

    if (t.dataLength != sizeof(t.buf)) {
      // 要发送的内容未占满 t.buf
      printf("dataLength = %d\n", t.dataLength);
    }

    if (t.dataLength == 0) {
      break;
    }

    // 发送 文件内容长度 和 文件内容
    ret = send(netFd, &t, sizeof(int) + t.dataLength, MSG_NOSIGNAL);
    ERROR_CHECK(ret, -1, "send");
  }

  // 结束的时候发送一个车厢为 0 的小火车
  t.dataLength = 0;
  send(netFd, &t, sizeof(int), MSG_NOSIGNAL);

  close(fd);

  return 0;
}
