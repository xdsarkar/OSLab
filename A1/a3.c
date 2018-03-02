#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

int main()
{
	FILE *fp; 
	fp = fopen("abc.txt", "r");
	char ch='a';
	pid_t pid; pid = fork();
	if(pid == 0){
		fscanf(fp, "%c", &ch); //if parent process doesn't read, thus child starts reading from 1st character
		printf("Child process: %c\n", ch); //child process reads the next character upto which parent process has read
	}
	else{
		printf("Parent process: %c\n", ch); //parent process then child process executes
		fclose(fp); //if fp: file pointer is closed child process cannot read using fp
	}
	return 0;
}


//Reason
/* If we wish to open a file, read a character, fork, and then have both parent and child process read the second character of the file independently.However, because both processes share a common file descriptor, one process might get the second character, and one might get the third. Furthermore, there is no guarantee the reads are atomic/non-atomic — the processes might get unpredictable results. */
