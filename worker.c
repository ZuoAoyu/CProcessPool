#include "head.h"

int makeChild(processData_t *pProcessData, int processNum) {
  pid_t pid;

  for (int i = 0; i < processNum; ++i) {
    pid = fork();
    if (pid == 0) {
      // 子进程
      handleEvent();
    } else {
      // 父进程
      pProcessData[i].pid = pid;
      pProcessData[i].status = FREE;
    }
  }

  return 0;
}

void handleEvent() {
  while (1)
    ;
}
