#include "head.h"

int epollCreate() {
  int epfd = epoll_create(1);
  if (epfd == -1) {
    perror("epoll_create");
    exit(-1);
  }

  return epfd;
}

int epollAdd(int fd, int epfd) {
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = fd;

  int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
  if (ret == -1) {
    perror("epoll_ctl add");
    exit(-1);
  }

  return 0;
}

int epollDel(int fd, int epfd) {
  int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
  if (ret == -1) {
    perror("epoll_ctl del");
    exit(-1);
  }

  return 0;
}
