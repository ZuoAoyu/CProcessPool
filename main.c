/**
 * 使用一个父进程创建若干个子进程
 * 目前，父进程只负责创建子进程，创建完成之后直接陷入死循环；
 * 子进程被创建之后也直接陷入死循环
 */
#include "head.h"

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "args failed.\n");
    fprintf(stderr, "usage: ./a.out 127.0.0.1 2338 10");
    exit(-1);
  }

  // 工作进程的数量
  int workerNum = atoi(argv[3]);

  // workerList 所有工作进程的状态
  processData_t *workerList =
      (processData_t *)calloc(sizeof(processData_t), workerNum);

  // 创建指定数量的子进程作为工作进程
  makeChild(workerList, workerNum);

  while (1)
    ;
}
