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
#include <setjmp.h>
#include <readline/readline.h>
#include <readline/history.h>

#define nl "\n"
#define EXIT "exit"
#define PROMPT "%s@%s :: $sash -> " ANSI_COLOR_RED "%s " ANSI_COLOR_RESET "-> "

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define STDIN 0
#define STDOUT 1

void tokenizeLogOr(char *);
void tokenizeLogAnd(char *);
void tokenizeSemiColon(char *);
int tokenizeSpace(char *);
void parseCommand(char *);
void parseIOF(char *);
void parseIOB(char *);
int parse_pipe(char *);
int piped_exec(char [][10], int);
int exec_function(char [][10], int, int);

static jmp_buf env;
void sigint_handler(int signo) { longjmp(env, 42); }

int main(int argc, char *argv[])
{
	printf("\033[H\033[J");
	signal(SIGINT, sigint_handler);
	int length = 0;
	char cwd[1024];
	char *command;
	char hostn[1204] = "";
	gethostname(hostn, sizeof(hostn));
	if(argc<2)
	{
		while(true)
		{
			if (sigsetjmp(env, 1) == 42)
			{
				printf("\n"); 
				continue;
			}
			if(getcwd(cwd, sizeof(cwd)) != NULL)
			{
				//command = readline(printf(PROMPT, getenv("LOGNAME"), hostn, cwd));
				printf("%s@%s :: " ANSI_COLOR_RED "%s " ANSI_COLOR_RESET, getenv("LOGNAME"), hostn, cwd);
				command = readline(":: $sash -> "); 
				fflush(stdout);
			}

			//fgets(command, 1024, stdin);

			if(strcmp(command, nl) == 0) continue;
			if(strcmp(command, "exit") == 0) exit(0);
			command[strlen(command)]='\0';
			add_history(command);
			parseCommand(command);
			memset(command, '\0', strlen(command));
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
	memset(command, '\0', strlen(command));
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
			else if(command[pos+1] == '>' && command[pos+2] == ' ')
			{
				stat = 5;
				break;
			}
			else if(command[pos+1] == '<' && command[pos+2] == ' ')
			{
				stat = 5;
				break;
			}
			else stat = 0;
		}
	}

	switch(stat)
	{
		case 0: tokenizeSpace(command); break;
		case 1: tokenizeSemiColon(command); break;
		case 2: tokenizeLogAnd(command); break;
		case 3: tokenizeLogOr(command); break;
		case 4: parse_pipe(command); break;
		case 5: parseIOF(command); break;
		case 6: parseIOB(command); break;
		default: printf("sash: Invalid command\n");
	}
}

int tokenizeSpace(char *command)
{
	int stat, flag = 0;
	int no_commands = 0;
	char subcommands[5][10];
	char *token = strtok(command, " ");
	while(token != NULL)
	{
		strcpy(subcommands[no_commands++], token);
		if(strcmp(token,"&") == 0) flag = 1;
		token = strtok(NULL, " ");
	}
	stat = exec_function(subcommands, no_commands, flag);
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

void parseIOF(char *command)
{
	int no_commands = 0;
	char subcommands[5][10];
	char *token = strtok(command, ">");
	while(token != NULL)
	{
		strcpy(subcommands[no_commands++], token);
		token = strtok(NULL, ">");
	}
    if(no_commands == 1) parse_pipe(subcommands[0]);
    else
    {
    	pid_t pid = fork();
    	if(pid == 0)
    	{
    		int fd = open(subcommands[1], O_CREAT | O_WRONLY, 0777);
    		dup2(fd, STDOUT);
    		close(fd);
    		if(parse_pipe(subcommands[0]) == EXIT_FAILURE) exit(EXIT_FAILURE);
    		exit(0);
    	}
	}
	memset(subcommands, '\0', 5*10*sizeof subcommands[0][0]);
	return;
}

void parseIOB(char *command)
{
	int no_commands = 0;
	char subcommands[5][10];
	char *token = strtok(command, "<");
	while(token != NULL)
	{
		strncpy(subcommands[no_commands++], token, strlen(token));
		token = strtok(NULL, "<");
	}
    if(no_commands == 1) parse_pipe(subcommands[0]);
    else
    {
    	pid_t pid = fork();
    	if(pid == 0)
    	{
    		int fd = open(subcommands[1], O_CREAT | O_WRONLY, 0777);
    		dup2(fd, STDIN);
    		close(fd);
    		if(parse_pipe(subcommands[0]) == EXIT_FAILURE) exit(EXIT_FAILURE);
    		exit(0);
    	}
	}
	memset(subcommands, '\0', 5*10*sizeof subcommands[0][0]);
	return;
}

int parse_pipe(char *command)
{
	int no_commands = 0, status;
	char subcommands[5][10];
	char *token = strtok(command, "|");
	while(token != NULL)
	{
		strcpy(subcommands[no_commands++], token);
		token = strtok(NULL, "|");
	}
	status = piped_exec(subcommands, no_commands-1);
	memset(subcommands, '\0', 5*10*sizeof subcommands[0][0]);
	return status;
}

int piped_exec(char command[][10], int num_pipe)
{
	if(num_pipe == 0) return tokenizeSpace(command[0]); //IO redirection >, <
	int pipefd[num_pipe*2];
	for(int i = 0; i < num_pipe ; i++)
	{
    	if(pipe(pipefd + i*2) < 0)
    	{
    		printf("sash: [pipe] error\n");
    		return EXIT_FAILURE;
    	}
	}

	for(int fd_i = 0; fd_i <= num_pipe; fd_i++)
	{
	    pid_t  pid = fork();
	    if(pid == 0)
	    {
	    	/* child gets input from the previous command, if it's not the first command */
	        if(fd_i != 0)
	        {
	        	if(dup2(pipefd[(fd_i*2)-2], STDIN) < 0)
	        	{
	        		printf("sash: [dup2] error\n");
	        		return EXIT_FAILURE; 
	        	}
	        }
	        //close(pipefd[(fd_i*2)-2]);
	        /* child outputs to next command, if it's not the last command */
	        if(fd_i != num_pipe)
	        {
	            if(dup2(pipefd[fd_i*2+1], STDOUT) < 0)
	        	{
	        		printf("sash: [dup2] error\n");
	        		return EXIT_FAILURE;
	        	}
	        }
	        //close(pipefd[fd_i*2+1]);
	        for(int k = 0; k<num_pipe*2; k++) close(pipefd[k]);
	    	tokenizeSpace(command[fd_i]);
	        exit(0);
	    }
	}
	for(int i=0; i<2*num_pipe;i++) close(pipefd[i]);
	for(int i=0; i<num_pipe+1; i++) wait(NULL);
}

int exec_function(char subcommands[][10], int no_commands, int flag)
{
	if(flag == 0)
	{
		if(strcmp(subcommands[0], "exit") == 0) exit(0);
		else if(strcmp(subcommands[0], "clear") == 0) printf("\033[H\033[J");
		else if(strcmp(subcommands[0], "cd") == 0) chdir(subcommands[1]);
		else if(strcmp(subcommands[0], "user") == 0) printf("Hello, %s\n", getenv("USER"));
		else
		{
			if(no_commands == 1)
			{
				int status;
				char *args[2] = {subcommands[0], NULL};
				pid_t pid;
				if ((pid = fork()) == 0) 
				{
					execvp(args[0], args);
					printf("sash: [%s] command failed\n", subcommands[0]);
					return 99; // Logical AND
				}
				wait(NULL);
				return 100;
			}
			else if(no_commands == 2)
			{
				int status;
				char *args[3] = {subcommands[0], subcommands[1], NULL};
				pid_t pid;
				int cstatus;
				if ((pid = fork()) == 0) 
				{
					execvp(args[0], args);
					printf("sash: [%s] command failed\n", subcommands[0]);
					return 99; // Logical AND
				}
				wait(NULL);
				return 100;
		    }
		}
	}
	else if(flag == 1)
	{
		if(no_commands == 2)
		{
			char *args[2] = {subcommands[0], NULL};
			pid_t pid;
			int cstatus;
			if ((pid = fork()) == 0) 
			{
				execvp(args[0], args);
				printf("sash: [%s] command failed\n", subcommands[0]);
				return 99; // Logical AND
			}
			printf("sash: [%s] pushed to background\n", subcommands[0]);
			wait(NULL);
			return 100; // Logical OR
		}
		else if(no_commands == 3)
		{
			char *args[3] = {subcommands[0], subcommands[1], NULL};
			pid_t pid;
			int cstatus;
			if ((pid = fork()) == 0) 
			{
				execvp(args[0], args);
				printf("sash: [%s] command failed\n", subcommands[0]);
				return 99; // Logical AND
			}
			printf("sash: [%s] pushed to background\n", subcommands[0]);
			wait(NULL);
			return 100; // Logical OR
	    }
	}
	return 0;
}