#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>

int main(int argc, char **argv)
{
	for(int i=1; i<argc; i++)
	{
		pid_t pid;
		char *myargvlist[]={argv[i],NULL};
		if((pid = fork()) == 0)
		{
			if(execve(myargvlist[0], myargvlist, NULL) == -1) //execve error
			{
				perror("Error: ");
				return 0;
			}
		}
		else if(pid > 0)
		{
			wait(NULL);
		}
	}
	return 0;
}