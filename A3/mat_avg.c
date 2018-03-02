#include <stdio.h> /*printf, scanf*/
#include <sys/ipc.h> /* inter process comm */
#include <sys/shm.h> /* shmdt, shmctl */
#include <sys/wait.h> /* wait */
#include <unistd.h> /*  fork, etc. */
#include <errno.h> /* perror */
#include <stdlib.h> /* exit */

int main()
{
	int m, n, res;
	pid_t pid;
	printf("Enter Matrix > Row x Column: ");
	scanf("%d%d", &m, &n);
    int A[m+2][n+2];
 	printf("Enter Elements of Matrix M[row][column]: \n");
 	for(int i = 0; i <= m+1; i++)
	{
    	for(int j = 0; j <= n+1; j++)
    	{
        	if(i==0 || j==0 || j==(n+1) || i==(m+1)) A[i][j]=0;
        	else scanf("%d",&A[i][j]);
    	}
	}

	for(int i = 0; i <= m+1; i++)
	{
    	for(int j = 0; j <= n+1; j++)
    	{
        	if(i==0 || j==0 || j==(n+1) || i==(m+1)) A[i][j]=0;
        	else printf("%d ",A[i][j]);
    	}
    	printf("\n");
	}

 	int shmid1 = shmget(IPC_PRIVATE, ((m+2)*(n+2))*sizeof(int), IPC_CREAT|0777);
 	if(shmid1 < 0)
 	{
 		perror("shmget() failure: ");
 		exit(1);
 	}

	for (int i = 1; i <= m; i++)
	{
    	for (int j = 1; j <= n; j++)
    	{
    		pid = fork();
    		if(pid==0)
    		{
    			int *childshm; childshm = (int *)shmat(shmid1, NULL, 0);
        		res = 0;
        		if((i==1 && j==1) || (i==m && j==1) || (i==1 && j==n) || (i==m && j==n))
        		{
            		res = (A[i-1][j-1] + A[i-1][j] + A[i-1][j+1] + A[i][j-1] + A[i][j] + A[i][j+1] + A[i+1][j-1] + A[i+1][j] + A[i+1][j+1])/4;
        		}
        		else if(i==1 || i==m || j==1 || j==n)
        		{
            		res = (A[i-1][j-1] + A[i-1][j] + A[i-1][j+1] + A[i][j-1] + A[i][j] + A[i][j+1] + A[i+1][j-1] + A[i+1][j] + A[i+1][j+1])/6;
        		}
        		else
        		{
            		res = (A[i-1][j-1] + A[i-1][j] + A[i-1][j+1] + A[i][j-1] + A[i][j] + A[i][j+1] + A[i+1][j-1] + A[i+1][j] + A[i+1][j+1])/9;
        		}
        		*(childshm+i+(j*n)) = res;
        		shmdt((void *)childshm); //detaches
        		exit(0);
    		}
    		else if(pid > 0) wait(NULL);
		}
	}

	printf("Resultant: \n");
	int *parentshm; parentshm = (int *)shmat(shmid1, NULL, 0);
	
	for(int s = 1; s <= m; s++)
	{
		for(int t = 1; t <= n; t++)
		{
    		printf("%d ", *(parentshm+s+(t*n)));
    	}
		printf("\n");
	}

	shmdt((void *)parentshm);
	shmctl(shmid1, IPC_RMID, NULL);
	return 0;
}
