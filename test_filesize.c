#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "args error\n");
    exit(-1);
  }

  int read_fd = open(argv[1], O_RDONLY);
  if (read_fd == -1) {
    perror("open failed.\n");
    exit(-1);
  }

  // 将文件指针移动到文件末尾；
  // 返回值是文件开头到当前指针位置的文件偏移量，
  // 也即文件大小
  off_t fileSize = lseek(read_fd, 0, SEEK_END);
  if (fileSize == (off_t)-1) {
    perror("lseek");
    close(read_fd);
    exit(-1);
  }
  printf("fileSize = %lld\n", (long long)fileSize);

  close(read_fd);
  return 0;
}
