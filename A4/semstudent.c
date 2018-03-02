#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/sem.h>

/* int semop(int semid, struct sembuf *sops, size_t nsops); */
#define P(s) semop(s, &POP, 1);
#define V(s) semop(s, &VOP, 1);

/* defining structure sembuf for 2 operations Pop/Vop */
struct sembuf POP;
struct sembuf VOP;

struct student
{
	int processid;
	int roll[2];
	int st;
};

int main()
{
	key_t key; key = ftok(".", 45); /* key generator */
	key_t keysem; keysem = ftok("..",57); /* key generator */
	int strength, total;
	struct student *reg;

	if(key == -1 || keysem == -1)
	{
		perror("ftok() failure: ");
		exit(1);
	}

	int shmid = shmget(key, sizeof(struct student), 0777 | IPC_CREAT);

	reg = (struct student *)shmat(shmid, NULL, 0);
	
	strength = reg->roll[0];
	total = reg->st;

    union semun 
    {
        int              val;    /* Value for SETVAL */
        struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
        unsigned short  *array;  /* Array for GETALL, SETALL */
        struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
    } setvalArg;

    setvalArg.val = 1;

    /* defining the operations pop/ vop */

    /* pop */
    POP.sem_num = 0;
    POP.sem_op = -1;
    POP.sem_flg = SEM_UNDO;

    /* vop */
    VOP.sem_num = 0;
    VOP.sem_op = 1;
    VOP.sem_flg = SEM_UNDO;

    int semid = semget(keysem, 1,IPC_CREAT | 0777 | IPC_EXCL); /* int semget(key_t key, int nsems, int semflg); */
	if(semid == -1) 
	{
        semid = semget(keysem, 1, IPC_CREAT | 0777);
        if(semid == -1) 
        {
            perror("semget() failed");
            exit(1);
        }
    } 
    else 
    {
        int status = semctl(semid, 0, SETVAL, setvalArg);
        if(status == -1) 
        {
            perror("semctl() failed");
            exit(1);
        }
    }
    
    P(semid)
	if(strength == 999) printf("Students left for attendance are 0\n");
	else if (strength != 999) printf("Students left for attendance are %d\n", strength);
	reg->processid = (int) getpid();
	if(strength == 999) printf("You can't put attendance, all are present!\n");
	else if((reg->roll[1]==-1) || (reg->roll[1]==998))
	{
		printf("Student process id: %d\n",reg->processid);
		printf("Enter attendance in the range <<1 : %d>>\n", total);
		printf("Roll: "); scanf("%d",&reg->roll[1]);
	}
	V(semid)

	int detach; detach=shmdt((void *)reg);
	return 0;
}