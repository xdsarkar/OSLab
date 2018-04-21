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
	int m, n, d, res, status=0;
	pid_t pid;
	printf("Enter d: "); scanf("%d", &d);
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

	key_t key1; key1 = ftok(".", 50); /* key generator */
	key_t key2; key2 = ftok("..",60); /* key generator */

    /* creating the shared memory memory segment in the parent. the shmid will be available in the child process too and the child process will use that shmid to attach shmat() the shared memory segment. */
 	int shmid1 = shmget(key1, (m*n)*sizeof(int), IPC_CREAT|0777);
 	int shmid2 = shmget(key2, 3*sizeof(int), IPC_CREAT|0777);
 	int *diff; diff = (int *)shmat(shmid2, NULL, 0);

 	if(shmid1 < 0 || shmid2 < 0)
 	{
 		perror("shmget() failure: ");
 		exit(1);
 	}

 	int count = 0;

 	do
 	{
 		*(diff+1) = (A[0][0] + A[0][1] + A[1][0] + A[1][1])/4;
 		*(diff+2) = (A[0][0] + A[0][1] + A[1][0] + A[1][1])/4;
 		++count;
		for (int i = 0; i < m; i++)
		{
	    	for (int j = 0; j < n; j++)
	    	{
	    		pid = fork();
	    		if(pid==0) /* child */
	    		{
	    			int *childshm; childshm = (int *)shmat(shmid1, NULL, 0);
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

	        		if(*(childshm+i+(j*n)) > *(diff+1)) *(diff+1) = *(childshm+i+(j*n));
	        		if(*(childshm+i+(j*n)) < *(diff+2)) *(diff+2) = *(childshm+i+(j*n));
	        		
	        		*diff = *(diff+1) - *(diff+2);
	        		int detach1 = shmdt((void *)childshm); //detaches
	        		if(detach1 == -1) perror("shmdt() failure: ");
	        		exit(0);
	    		}
	    		else if(pid > 0) wait(&status);
			}
		}
		int *childshm; childshm = (int *)shmat(shmid1, NULL, 0);
		for(int i=0; i<m; i++)
		{
			for (int j=0; j<n; j++)
			{
				A[i][j] = *(childshm+i+(j*n));
			}
		}
	}while((*(diff))>=d);

	int detach2 = shmdt((void *)diff);
	if(detach2 == -1) perror("shmdt() failure: ");

	printf("Successive Averaging: %d", count);
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
	shmctl(shmid2, IPC_RMID, NULL);
	exit(0);
}
