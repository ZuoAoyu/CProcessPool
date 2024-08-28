#include "head.h"

int tcpInit(char *ip, char *port, int *pSockFd) {
  *pSockFd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip);
  addr.sin_port = htons(atoi(port));

  int ret;

  // 设置 socket 选项，端口重用
  int reuse = 1;
  ret = setsockopt(*pSockFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  if (ret == -1) {
    perror("setsockopt");
    exit(-1);
  }

  ret = bind(*pSockFd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if (ret == -1) {
    perror("bind");
    exit(-1);
  }

  listen(*pSockFd, 10);

  return 0;
}
