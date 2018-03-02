#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

struct student
{
	int processid;
	int roll[2];
	int st;
};

int main()
{
	key_t key; key = ftok(".", 45);
	int strength, total;
	struct student *reg;

	if(key == -1)
	{
		perror("ftok() failure: ");
		exit(1);
	}

	int shmid = shmget(key, sizeof(struct student), 0777 | IPC_CREAT);
	reg = (struct student *)shmat(shmid, NULL, 0);
	
	strength = reg->roll[0];
	total = reg->st;

	if(strength == 999) printf("Teacher is not taking attendance!\n");
	else if (strength != 999) printf("Students left for attendance are %d\n", strength);
	reg->processid = (int) getpid();
	if(strength == 999) printf("You can't put attendance, stop!\n");
	else if((reg->roll[1]==-1) || (reg->roll[1]==998))
	{
		printf("Student process id: %d\n",reg->processid);
		printf("Enter attendance in the range <<1 : %d>>\n", total);
		printf("Roll: "); scanf("%d",&reg->roll[1]);
	}
	int detach; detach=shmdt((void *)reg);
	return 0;
}