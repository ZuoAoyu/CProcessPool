#ifndef __HEAD_H
#define __HEAD_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum workerStatus { FREE, BUSY };

typedef struct {
  pid_t pid;  // pid of working process
  int status; // status of working process
} processData_t;

void handleEvent();
int makeChild(processData_t *pProcessData, int processNum);

#endif
