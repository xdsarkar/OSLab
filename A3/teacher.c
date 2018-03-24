#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<string.h>
#include <signal.h>
#define max 50 //max strength

int shmid,n;
int arrglob_roll[max] = {0};
int arrglob_id[max] = {0};
/* Following is a signal handler for the keypress ^C, that is, ctrl-c */
typedef void (*sighandler_t)(int);

void releaseSHM(int signum) 
{
	printf("\n***** Attendance Register *****\n");
	for(int i=1; i<=n; i++)
	{
		if(arrglob_roll[i]==1) printf("<<Student Roll: %d>> || <<Process ID: %d>> PRESENT\n",i, arrglob_id[i]);
		else printf("<<Student Roll: %d>> ABSENT\n",i);
	}
	printf("\n");
    int status;
    // int shmctl(int shmid, int cmd, struct shmid_ds *buf);
    status = shmctl(shmid, IPC_RMID, NULL); /* IPC_RMID is the command for destroying the shared memory*/
    if (status == 0) fprintf(stderr, "Remove shared memory with id = %d.\n", shmid);
    else if (status == -1) fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
    else fprintf(stderr, "shmctl() returned wrong value while removing shared memory with id = %d.\n", shmid);

    // int kill(pid_t pid, int sig);
    status = kill(0, SIGKILL);
    if (status == 0) fprintf(stderr, "Kill successful\n");
    else if (status == -1) 
    {
        perror("Kill failure: \n");
        fprintf(stderr, "Cannot remove shared memory with id = %d.\n", shmid);
    }
    else fprintf(stderr, "Kill(2) returned wrong value.\n");
}

struct student
{
	int processid; //pid
	int roll[2]; //strength, roll
	int st; //total
};

int main()
{
	key_t key;
	printf("Enter number of students in register: "); scanf("%d",&n);

	int arr_roll[n];
	int arr_id[n];

	memset(arr_roll, 0, sizeof(int));
	memset(arr_id, 0, sizeof(int));

	struct student *reg; /* pointer to struct student */
	key = ftok(".", 45); /* key generator */

	if(key == -1)
	{
		perror("ftok() failure: ");
		exit(1);
	}

	/* signal handler */
	sighandler_t shandler;
	shandler =  signal(SIGINT, releaseSHM);
	
	/* size of shared mem =  struct student */
	shmid = shmget(key, sizeof(struct student), 0777 | IPC_CREAT);
	reg = (struct student *)shmat(shmid, NULL, 0);
	printf("<<Enter from 1 : %d>>\n",n);
	reg->st = n; /* total */
	reg->roll[0] = n; /* strength left for attendance */
	reg->roll[1] = -1; /* initially -1 */
	int attended = 0; /* count of attended */
	/* Student Roll == 999, loop exits) */
	do
	{
		if(((reg->roll[1]) != -1) && ((reg->roll[1]) != 999))
		{   
			if((reg->roll[1])!=998) /* If invalid roll or replica of roll, set val to 998 */
			{	
				if(!(((reg->roll[1])>=1) && ((reg->roll[1])<=n))) /* not in range */
				{
					printf("Select roll from range mentioned\n");
					reg->roll[1]=998;
				}
				else if(arr_roll[(reg->roll[1])]==1) /* if roll again given */
				{
					printf("Student with roll %d, shut up!\n", reg->roll[1]);
					reg->roll[1]=998;
				}
				else if(((reg->roll[1])>=1) && ((reg->roll[1])<=n))
				{
					printf("Student with roll %d is present with pid = %d\n",reg->roll[1], reg->processid);
					/*stores in an array */
					arr_roll[reg->roll[1]]=1;
					arrglob_roll[reg->roll[1]]=1;
					arr_id[reg->roll[1]]=reg->processid;
					arrglob_id[reg->roll[1]]=reg->processid;
					/* above */
					reg->roll[0]=reg->roll[0]-1; /* decreases strength on valid roll */
					reg->roll[1]=-1;
					attended++;
					if(attended == n)
					{
						reg->roll[0] = 999;
						printf("\n*****All students are present, today!*****\n");
						for(int i=1; i<=n; i++)
						{	
							if(arr_roll[i]==1) printf("<<Student Roll: %d>> || <<Process ID: %d>> PRESENT\n",i, arr_id[i]);
							else printf("<<Student Roll: %d>> ABSENT\n",i);
						}
						return 0;
					}
				}
			}
		}
	}while((reg->roll[1])!=999); /* loop exits if reg->roll[1] == 999 */

	/* Prints if Roll == 999*/
	if((reg->roll[1]) == 999)
	{
		printf("\n*****Attendence Register*****\n");
		for(int i=1; i<=n; i++)
		{
			if(arr_roll[i] == 1) printf("<<Student Roll: %d>> || <<Process ID: %d>> PRESENT\n",i, arr_id[i]);
			else printf("<<Student Roll: %d>> ABSENT\n",i);
		}
	}
	int detach; detach=shmdt((void *)reg);
	shmctl(shmid, IPC_RMID, NULL);
	return 0;	
}