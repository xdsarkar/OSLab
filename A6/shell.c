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
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>

#define nl "\n"
#define EXIT "exit"
#define PROMPT "%s@%s :: $sash -> " ANSI_COLOR_RED "%s " ANSI_COLOR_RESET "-> "

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

void tokenizeLogOr(char *);
void tokenizeLogAnd(char *);
void tokenizeSemiColon(char *);
int tokenizeSpace(char *);
void parseCommand(char *);
//void pipeHandler(char *);
int exec_function(char [][10], int, int);

void sigkill(int p)
{
    signal(SIGINT,&sigkill);  
    kill(my_pid,SIGKILL);
    fflush(stdout);
}

int main(int argc, char *argv[])
{
	printf("\033[H\033[J");
	int length = 0;
	char cwd[1024];
	char command[1024];
	char hostn[1204] = "";
	gethostname(hostn, sizeof(hostn));
	if(argc<2)
	{
		while(true)
		{
			if(getcwd(cwd, sizeof(cwd)) != NULL)
			{
				printf(PROMPT, getenv("LOGNAME"), hostn, cwd); 
				fflush(stdout);
			}
			fgets(command, 1024, stdin);
			if(strcmp(command, nl) == 0) continue;
			command[strlen(command)-1]='\0';
			parseCommand(command);
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
		parseCommand(command);
	}
}

void parseCommand(char *command)
{
	int pos, stat = 0;
	for(pos = 0; pos <= strlen(command); pos++) 
	{
		if(command[pos] == ' ')
		{
			if(command[pos+1] == ';')
			{
				stat = 1;
				break;
			}
			else if(command[pos+1] == '&' && command[pos+2] == '&')
			{
				stat = 2;
				break;
			}
			else if(command[pos+1] == '|' && command[pos+2] == '|')
			{
				stat = 3;
				break;
			}
			else if(command[pos+1] == '|' && command[pos+2] == ' ')
			{
				stat = 4;
				break;
			}
			else
			{
				stat = 0;
			}
		}
	}

	switch(stat)
	{
		case 0: tokenizeSpace(command); break;
		case 1: tokenizeSemiColon(command); break;
		case 2: tokenizeLogAnd(command); break;
		case 3: tokenizeLogOr(command); break;
		case 4: //pipeHandler(command); break;
		default: printf("sash: Invalid command\n");
	}
}

int tokenizeSpace(char *command)
{
	int stat;
	int no_commands = 0;
	char subcommands[5][10];
	char *token = strtok(command, " ");
	while(token != NULL)
	{
		strcpy(subcommands[no_commands++], token);
		token = strtok(NULL, " ");
	}
	stat = exec_function(subcommands, no_commands, 0);
	memset(subcommands, '\0', 5*10*sizeof subcommands[0][0]);
	if(stat == 99) return 99; // Logical AND
	if(stat == 100) return 100; // Logical OR
}

void tokenizeSemiColon(char *command)
{
	int no_commands = 0;
	char subcommands[5][10];
	char *token = strtok(command, ";");
	while(token != NULL)
	{
		strcpy(subcommands[no_commands++], token);
		token = strtok(NULL, ";");
	}
	for(int i=0; i<no_commands; i++)
	{
		tokenizeSpace(subcommands[i]);	
	}
	memset(subcommands, '\0', 5*10*sizeof subcommands[0][0]);
}

void tokenizeLogAnd(char *command)
{
	int stat;
	int no_commands = 0;
	char subcommands[5][10];
	char *token = strtok(command, "&&");
	while(token != NULL)
	{
		strcpy(subcommands[no_commands++], token);
		token = strtok(NULL, "&&");
	}
	for(int i=0; i<no_commands; i++)
	{
		stat = tokenizeSpace(subcommands[i]);
		if(stat == 99) break;	
	}
	memset(subcommands, '\0', 5*10*sizeof subcommands[0][0]);
}

void tokenizeLogOr(char *command)
{
	int stat;
	int no_commands = 0;
	char subcommands[5][10];
	char *token = strtok(command, "||");
	while(token != NULL)
	{
		strcpy(subcommands[no_commands++], token);
		token = strtok(NULL, "||");
	}
	for(int i=0; i<no_commands; i++)
	{
		stat = tokenizeSpace(subcommands[i]);
		if(stat == 100) break;	
	}
	memset(subcommands, '\0', 5*10*sizeof subcommands[0][0]);
}

int exec_function(char subcommands[][10], int no_commands, int flag)
{
	if(strcmp(subcommands[0], "pwd") == 0)
	{
		char cwd[1024];
   		if(getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n", cwd);
	}
	else if(strcmp(subcommands[0], "exit") == 0)
	{
		exit(0);
	}
	else if(strcmp(subcommands[0], "clear") == 0)
	{
		printf("\033[H\033[J");
	}
	else if(strcmp(subcommands[0], "cd") == 0)
	{
		chdir(subcommands[1]);
	}
	else if(strcmp(subcommands[0], "user") == 0)
	{
		printf("Hello, %s\n", getenv("USER"));
	}
	else
	{
		if(no_commands == 1)
		{
			char *args[2] = {subcommands[0], NULL};
			pid_t pid;
			if ((pid = fork()) == 0) 
			{
				execvp(args[0], args);
				printf("sash: [%s] command failed\n", subcommands[0]);
				return 99; // Logical AND
			}
			else if(pid > 0) 
			{
				wait(NULL);
				return 100; // Logical OR
			}
		}
		else if(no_commands == 2)
		{
			char *args[3] = {subcommands[0], subcommands[1], NULL};
			pid_t pid;
			if ((pid = fork()) == 0) 
			{
				execvp(args[0], args);
				printf("sash: [%s] command failed\n", subcommands[0]);
				return 99; // Logical AND
			}
			else if(pid > 0) 
			{
				wait(NULL);
				return 100; // Logical OR
			}
	    }
	}
	return 0;
}