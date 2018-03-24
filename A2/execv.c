#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<unistd.h>

int main(int argc, char **argv)
{
	fork() || fork() && fork();
	printf("Child\n");
	return 0;
}