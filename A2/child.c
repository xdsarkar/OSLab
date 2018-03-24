#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>

int main(int argc, int **argv)
{
	printf("Child PID = %ld\n", (long) getpid());
	pause();
	return 0;
}