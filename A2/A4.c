#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<sys/types.h>

void termination(int status)
{
    if (WIFEXITED(status)) {
        printf("Exited, status = %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Killed by signal = %d\n", WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
        printf("Stopped by signal = %d\n", WSTOPSIG(status));
    } else if (WIFCONTINUED(status)) {
        printf("Continuead\n");
    }
    //exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) //take argv from shell
{
	for(int i=1; i<argc; i++)
	{
		char *myargv[]={argv[i],NULL};
		pid_t child_pid, wpid;
		int status;

		if((child_pid = fork())==0)
		{
			if(execve(argv[i], myargv, NULL) == -1) //execve error
			{
				perror("execve failed(): ");
				exit(1);
			}
			else
			{
				execve(argv[i], myargv, NULL);
			}
		}
		else if(child_pid>0)
		{
			wpid = waitpid(child_pid, &status, WUNTRACED | WCONTINUED); //WUTRACED and WCONTINUED are used for use of proper kill execuation from shell
			termination(status); //status/ signal determination
			printf("\n");
		}
	}
	return 0;
}

/* if .txt is read, execve returns error, i.e., -1, as abc.txt is not an executable*/
