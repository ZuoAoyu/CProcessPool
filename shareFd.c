#include "head.h"

// 发送 文件对象 的"访问权"
int sendFd(int pipeFd, int fdToSend, int exitFlag) {
  struct msghdr hdr;
  /* memset 的作用：
    msg_name = NULL;
    msg_namelen = 0; */
  memset(&hdr, 0, sizeof(struct msghdr)); // 不可省略
  // 无论正文部分在业务中是否有用，必须要有正文：
  struct iovec iov[1];          // 一组离散的消息
  iov[0].iov_base = &exitFlag;  // 第一个消息的首地址
  iov[0].iov_len = sizeof(int); // 第一个消息的长度
  // 将离散的消息放入 hdr 中
  hdr.msg_iov = iov;  // 正文消息
  hdr.msg_iovlen = 1; // 正文消息数量：1 条

  // 控制字段部分，填入要发送的内核数据结构（的文件描述符，相当于指针的作用了）
  struct cmsghdr *pcmsghdr = (struct cmsghdr *)calloc(1, CMSG_LEN(sizeof(int)));
  pcmsghdr->cmsg_len = CMSG_LEN(sizeof(int));
  pcmsghdr->cmsg_level = SOL_SOCKET;
  pcmsghdr->cmsg_type = SCM_RIGHTS;
  // 通过 pcmsghdr 得到它的 data 成员的首地址，
  // 强转成 int*，再解引用
  *(int *)CMSG_DATA(pcmsghdr) = fdToSend;

  // 将控制字段信息放入 hdr 中
  hdr.msg_control = pcmsghdr;
  hdr.msg_controllen = CMSG_LEN(sizeof(int));

  // 发送消息
  int ret = sendmsg(pipeFd, &hdr, 0);
  if (ret == -1) {
    perror("sendmsg");
    exit(-1);
  }

  return 0;
}

// 接收 文件对象 的"访问权"
int recvFd(int pipeFd, int *pfdtorecv, int *exitFlag) {
  // 接收和发送的区别：接收方不知道 buf 和 pcmsghdr 的内容
  struct msghdr hdr;
  /* memset 的作用：
    msg_name = NULL;
    msg_namelen = 0; */
  memset(&hdr, 0, sizeof(struct msghdr)); // 不可省略
  // 正文部分，结构和 sendmsg 一致
  struct iovec iov[1];          // 一组离散的消息
  iov[0].iov_base = exitFlag;   // 第一个消息的首地址
  iov[0].iov_len = sizeof(int); // 第一个消息的长度
  // 将离散的消息放入 hdr 中
  hdr.msg_iov = iov;  // 正文消息
  hdr.msg_iovlen = 1; // 正文消息数量：1 条

  // 控制字段部分
  struct cmsghdr *pcmsghdr = (struct cmsghdr *)calloc(1, CMSG_LEN(sizeof(int)));
  pcmsghdr->cmsg_len = CMSG_LEN(sizeof(int));
  pcmsghdr->cmsg_level = SOL_SOCKET;
  pcmsghdr->cmsg_type = SCM_RIGHTS;
  // 将控制字段信息放入 hdr 中
  hdr.msg_control = pcmsghdr;
  hdr.msg_controllen = CMSG_LEN(sizeof(int));

  // 接收消息
  int ret = recvmsg(pipeFd, &hdr, 0);
  if (ret == -1) {
    perror("sendmsg");
    exit(-1);
  }

  // 通过 pcmsghdr 得到它的 data 成员的首地址，强转成 int*，再解引用
  *pfdtorecv = *(int *)CMSG_DATA(pcmsghdr);

  printf("exitFlag = %d, fd = %d\n", *exitFlag, *pfdtorecv);

  return 0;
}
