#include <stdio.h> /* printf, scanf */
#include <sys/ipc.h> /* inter process comm */
#include <sys/shm.h> /* shmdt, shmctl */
#include <sys/wait.h> /* wait */
#include <unistd.h> /*  fork, etc. */
#include <errno.h> /* perror */
#include <stdlib.h> /* exit */
#include <string.h> /* memset */

int main()
{
	int m, n, res, status=0;
	pid_t pid;
	printf("Enter Matrix >> nRow >> nCols : "); scanf("%d%d", &m, &n);
    int A[m][n];
 	printf("Enter Elements of Matrix A[row][column]: \n");
 	for(int i = 0; i < m; i++)
	{
    	for(int j = 0; j < n; j++)
    	{
			scanf("%d",&A[i][j]);
    	}
	}

	printf("\nThe Matrix:\n");
	for(int i = 0; i < m; i++)
	{
    	for(int j = 0; j < n; j++)
    	{
        	printf("%d ",A[i][j]);
    	}
    	printf("\n");
	}
    
    /*
    1 2 3 4 5
    6 7 8 9 0
    0 9 8 7 6
    5 4 3 2 1 
    */	

    /* creating the shared memory memory segment in the parent. the shmid will be available in the child process too and the child process will use that shmid to attach shmat() the shared memory segment. */
 	int shmid1 = shmget(IPC_PRIVATE, (m*n)*sizeof(int), IPC_CREAT|0777);
    int *childshm; childshm = (int *)shmat(shmid1, NULL, 0);
    
 	if(shmid1 < 0)
 	{
 		perror("shmget() failure: ");
 		exit(1);
 	}

	for (int i = 0; i < m; i++)
	{
    	for (int j = 0; j < n; j++)
    	{
    		pid = fork();
    		if(pid==0) /* child */
    		{
        		res = 0;
        		if((i==0 && j==0) || (i==m-1 && j==0) || (i==0 && j==n-1) || (i==m-1 && j==n-1))
        		{
        			if(i==0 && j==0) res = (A[i][j] + A[i][j+1] + A[i+1][j] + A[i+1][j+1])/4;
        			if(i==0 && j==n-1) res = (A[i][j] + A[i][j-1] + A[i+1][j] + A[i+1][j-1])/4;
        			if(i==m-1 && j==0) res = (A[i][j] + A[i][j+1] + A[i-1][j] + A[i-1][j+1])/4;
        			if(i==m-1 && j==n-1) res = (A[i][j] + A[i][j-1] + A[i-1][j] + A[i-1][j-1])/4;
        		}
        		else if(i==0 || i==m-1 || j==0 || j==n-1)
        		{
        			if(i==0) res = (A[i][j-1] + A[i][j] + A[i][j+1] + A[i+1][j-1] + A[i+1][j] + A[i+1][j+1])/6;
        			if(i==m-1) res = (A[i-1][j-1] + A[i-1][j] + A[i-1][j+1] + A[i][j-1] + A[i][j] + A[i][j+1])/6;
        			if(j==0) res = (A[i-1][j] + A[i-1][j+1] + A[i][j] + A[i][j+1] + A[i+1][j] + A[i+1][j+1])/6;
        			if(j==n-1) res = (A[i-1][j-1] + A[i-1][j] + A[i][j-1] + A[i][j] + A[i+1][j-1] + A[i+1][j])/6;
        		}
        		else res = (A[i-1][j-1] + A[i-1][j] + A[i-1][j+1] + A[i][j-1] + A[i][j] + A[i][j+1] + A[i+1][j-1] + A[i+1][j] + A[i+1][j+1])/9;
        		*(childshm+i+(j*n)) = res;
        		int detach1 = shmdt((void *)childshm); //detaches
        		if(detach1 == -1) perror("shmdt() failure: ");
        		exit(0);
    		}
    		else if(pid > 0) wait(&status);	
		}
	}

	printf("\nResultant Matrix: \n");
	int *parentshm; parentshm = (int *)shmat(shmid1, NULL, 0);
	for(int s = 0; s < m; s++)
	{
		for(int t = 0; t < n; t++)
		{
    		printf("%d ", *(parentshm+s+(t*n)));
    	}
		printf("\n");
	}
	int detach2 = shmdt((void *)parentshm);
	if(detach2 == -1) perror("shmdt() failure");
	shmctl(shmid1, IPC_RMID, NULL);
	exit(0);
}