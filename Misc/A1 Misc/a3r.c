#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

int main() {
    int status = 0;
    char ch;
    FILE *fp = fopen("abc.txt","w+");

    ch='x';
    fputc(ch,fp);
    /*
    ch=fgetc(fp);
    if(ch == '\0') printf("Root: Read Failed\n");
    else printf("Root Process: %c\n",ch);
	*/
    pid_t pid = fork();
    if (pid == 0)
    {
        // child process
        ch=fgetc(fp);
        if(ch == '\0') printf("Child: Read Failed\n");
        else printf("Child Process: %c\n",ch);
    }
    else if (pid > 0)
    {
        // parent process
        ch=fgetc(fp);
        if(ch == '\0') printf("Parent: Read Failed\n");
        else printf("Parent Process: %c\n",ch);
        fclose(fp);
    }
    return 0;
}