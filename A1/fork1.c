#include<stdio.h>
#include<unistd.h>

int main()
{
	if(fork() == 0) printf("Child ");
	else printf("Parent ");
	return 0;
}
