#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, int **argv)
  pid_t pid;
  char *myargvlist[]={argv[1],argv[2],NULL};
  if ((pid = fork()) == -1) perror("fork error");
  else if (pid == 0)
  {
    execve(myargvlist[0], myargvlist, NULL);
    printf("Return not expected. Must be an execv error.n");
  }
  return 0;
}
