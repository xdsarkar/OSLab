#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<unistd.h>

int main(int argc, char **argv)
{
	int i=0;
	pid_t pid;
	printf("Hello World, Root!\n");
	pid = fork();
	if(pid == 0)
	{
		execvp("./child",argv);
	}
	if(pid > 0) 
	{
		wait(NULL);
		printf("Parent Parent Parent\n");
	}
	return 0;
}