#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

char input_buffer[1024];
int flag, len;
char cwd[1024];
char current_directory[1000];

void clear_shell()
{
	flag=0;
	len=0;
	input_buffer[0]='\0';
	cwd[0] = '\0';
}

void prompt()
{
	char shell[1024];
	shell[0] = '\0';
	strcpy(shell, "myfs");
	strcat(shell, "> ");
	printf("%s", shell);
}

int main()
{
	int status;
	char nl[2]={"\n"};
	getcwd(current_directory, sizeof(current_directory));
	while(1)
	{
		clear_shell();
		prompt();
		fgets(input_buffer, 1024, stdin);
		if(strcmp(input_buffer, nl)==0) continue;
		len = strlen(input_buffer);
      	input_buffer[len-1]='\0';
      	if(strcmp(input_buffer, "exit") == 0) 
        {
        	flag = 1;
        	break;
        }
        if(flag == 1) return 0;
	}
	return 0;
}