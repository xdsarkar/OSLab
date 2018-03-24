#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>
#include <stdbool.h>

#define nl "\n"
#define EXIT "exit"
#define PROMPT "$sash> "

void tokenize(char *);
int internal_function(char [][10], int, int);	

int main(int argc, char **argv)
{
	int length = 0;
	char command[1024];
	if(argc<2)
	{
		while(true)
		{
			printf(PROMPT); fflush(stdout);
			fgets(command, 1024, stdin);
			if(strcmp(command, nl) == 0) continue;
			command[strlen(command)-1]='\0';
			tokenize(command);
		}
	}
	if(argc>=2) /* command line */
	{
		for(int i=1; i<argc; i++) length = length + strlen(argv[i]);
		length = length + (argc - 1);
		for (int i = 1; i < argc; ++i)
		{
			strcat(command, argv[i]);
			strcat(command, " ");
		}
		command[length] = '\0';
		tokenize(command);
	}
}

void tokenize(char *command)
{
	int no_commands = 0;
	char subcommands[5][10];
	char *token = strtok(command, " ");
	while(token != NULL)
	{
		strcpy(subcommands[no_commands++], token);
		token = strtok(NULL, " ");
	}
	internal_function(subcommands, no_commands, 0);
}

int internal_function(char subcommands[][10], int no_commands, int flag)
{
	if(strcmp(subcommands[0], "pwd") == 0)
	{
		char cwd[1024];
   		if(getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n", cwd);
	}
	if(strcmp(subcommands[0], "exit") == 0)
	{
		exit(0);
	}
	if(strcmp(subcommands[0], "clear") == 0)
	{
		printf("\033[H\033[J");
	}
	if(strcmp(subcommands[0], "cd") == 0)
	{
		chdir(subcommands[1]);
	}
	if(strcmp(subcommands[0], "user") == 0)
	{
		printf("Hello, %s\n", getenv("USER"));
	}
	return 0;
}