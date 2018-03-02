#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, int **argv)
  pid_t pid;
  if ((pid = fork()) == -1) perror("fork error");
  else if (pid == 0)
  {
    execv("~/bin/ls", parmList);
    printf("Return not expected. Must be an execv error.n");
  }
  return 0;
}
