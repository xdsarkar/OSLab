#include<stdio.h>
#include <unistd.h> 

int main()
{
	int count1=0,count2=0;
	for(int i=0; i<2; i++)
	{
		pid_t pid = fork();
		if(pid == 0) count1++;
		else if(pid > 0) count2++;
	}
	int count = count1 + count2;
	printf("count1=%d, Count2=%d, Total=%d \n",count1,count2,count);
	return 0;
}
