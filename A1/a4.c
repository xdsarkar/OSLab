#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

int main()
{
	FILE *fp; 
	fp = fopen("abc.txt", "w"); //write
	char ch='a';
	pid_t pid; pid = fork();
	if(pid == 0){
		fprintf(fp,"wer");
		printf("Child process: %c\n", ch); //child process writes the next character upto which parent process has written
	}
	else{
		printf("Parent process: %c\n", ch); //parent process then child process executes
		fprintf(fp,"werd");
		fclose(fp); //if fp: file pointer is closed parent & child process cannot read using fp
	}
	return 0;
}
