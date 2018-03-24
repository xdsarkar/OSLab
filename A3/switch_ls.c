#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<string.h>

int main(int argc, char *argv[])
{
	pid_t wpid, child_pid;
	int status;
	child_pid = fork();
	if(child_pid==0)
	{
		if(argc == 2) //suppose -l
		{
			char *const exx[]={argv[1],NULL};
			execve(exx[0], exx, NULL);
		}
		else if(argc == 3 && argv[2][0]=='-' && argv[2][2]=='\0') //suppose ls -l
		{
			char *const ex1[]={argv[1],argv[2],NULL};
			switch(argv[2][1])
			{
				case 'l':
					execve(ex1[0], ex1, NULL); 
					break;
				case 'a':
					execve(ex1[0], ex1, NULL); 
					break;
				case 'A':
					execve(ex1[0], ex1, NULL); 
					break;
				case 'b':
					execve(ex1[0], ex1, NULL); 
					break;
				case 'B':
					execve(ex1[0], ex1, NULL); 
					break;
				case 'c':
					execve(ex1[0], ex1, NULL); 
					break;
				case 'C':
					execve(ex1[0], ex1, NULL); 
					break;
				case 'd':
					execve(ex1[0], ex1, NULL); 
					break;
				case 'D':
					execve(ex1[0], ex1, NULL); 
					break;
				default:
					printf("Invalid: Type man ls to see options\n");
					break;
			}
		}
		else if(argc>3) //suppose ls -l -a
		{
			char *destination = argv[2];
			int j=2;
			for(int i=3; i<argc; i++)
			{
				destination[j] = argv[i][1];
				j++;
			}
			destination[j]='\0';
			char *const ex2[] = {argv[1], destination, NULL};
			execve(ex2[0], ex2, NULL); 
		}
	}
	else if(child_pid>0)
	{
		wpid = waitpid(child_pid, &status, 0);
	}
	return 0;
}