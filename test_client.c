#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "args failed.\n");
    exit(-1);
  }

  int sockFd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(argv[1]);
  addr.sin_port = htons(atoi(argv[2]));

  int ret =
      connect(sockFd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if (ret == -1) {
    perror("connect");
    exit(-1);
  }

  char buf[1024] = {0};
  read(STDIN_FILENO, buf, sizeof(buf));
  send(sockFd, buf, strlen(buf), 0);

  bzero(buf, sizeof(buf));
  recv(sockFd, buf, sizeof(buf), 0);
  printf("recv: %s\n", buf);

  close(sockFd);

  return 0;
}
