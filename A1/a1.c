#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include <sys/types.h> //wait()
#include <sys/wait.h> //wait()
int main()
{
	pid_t pid;
	printf("Enter n to create n child processes: ");
	int n; scanf("%d",&n);
	for(int i=0; i<n; i++)
	{
		pid = fork();
		if(pid == 0)
		{
			printf("(son) process_id = %d, (parent) process_id = %d\n", getpid(),getppid());
			/*getppid(): returns processs_id of the parent*/
			/*getpid(): returns process_id of the child*/
			exit(0); //calling exit() is a good way to cause pending stdio buffers to be flushed twice.
		}
		wait(NULL); /*wait() is made, the caller will be blocked until one of its child processes exits*/
	}

	return 0;
}