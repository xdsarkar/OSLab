#include <stdio.h>			// standard input/output
#include <sys/ipc.h>        // shmget(), shmctl()
#include <sys/shm.h>        // shmget(), shmat(), shmdt(), shmctl()
#include <sys/types.h>      // shmat(), shmdt()
#include <stdlib.h>         // exit()
#include <stdbool.h>		// bool
#include <string.h>
#include <sys/sem.h>
#define MAX_STACK 10

typedef struct {
	key_t 	stackKey;			// key for the shared memory of a stack
	int 	stackdescid;		// shared stack memory segment id
	int 	data_size;			// data element size for this stack
	int 	stack_size;			// size for this stack
	void 	*stack_top;			// pointer to this stack
	int 	elements_no;		// present no of elements
	bool 	free;				// whether this descriptor is free for use
}stackdesc;

bool owner = false;
stackdesc *temp;
int shmid;

#define NO_SEM  1
#define P(s) semop(s, &Pop, 1);
#define V(s) semop(s, &Vop, 1);

struct sembuf Pop;
struct sembuf Vop;

	int shmid, semid, status;

    union semun {
        int              val;    /* Value for SETVAL */
        struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
        unsigned short  *array;  /* Array for GETALL, SETALL */
        struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
    } setvalArg;

    setvalArg.val = 1;


    /* struct sembuf has the following fields */
    //unsigned short sem_num;  /* semaphore number */
    //short          sem_op;   /* semaphore operation */
    //short          sem_flg;  /* operation flags */

    Pop.sem_num = 0;
    Pop.sem_op = -1;
    Pop.sem_flg = SEM_UNDO;

    Vop.sem_num = 0;
    Vop.sem_op = 1;
    Vop.sem_flg = SEM_UNDO;

stackdesc* shstackget(key_t key, int element_size, int stack_size) {
	key_t mykey;
	
    //key_t ftok(const char *pathname, int proj_id);
    mykey = ftok("/home/saptarshi/Desktop/OS/assgn5/2.c", 1);

    // int semget(key_t key, int nsems, int semflg);
    semid = semget(mykey, NO_SEM, IPC_CREAT | IPC_EXCL | 0777);
    if(semid == -1) {
        
        // int semget(key_t key, int nsems, int semflg);
        semid = semget(mykey, NO_SEM, IPC_CREAT | 0777);
        if(semid == -1) {
            perror("semget() failed");
            exit(1);
        }
    } else {
        
        // int semctl(int semid, int semnum, int cmd, ...);
        status = semctl(semid, 0, SETVAL, setvalArg);
        if(status == -1) {
            perror("semctl() failed");
            exit(1);
        }
    }

	// int shmget(key_t key, size_t size, int shmflg);
	shmid =  shmget(mykey, sizeof(stackdesc) * MAX_STACK, IPC_CREAT | IPC_EXCL | 0777);
	if (shmid == -1) { 

		// int shmget(key_t key, size_t size, int shmflg);
		shmid =  shmget(mykey, sizeof(stackdesc) * MAX_STACK, IPC_CREAT | 0777);
        if (shmid == -1) {
        	perror("shmget failed: ");
        	return (void*)-1;
        }
    } else {
    	owner = true;
    	
    	temp = (stackdesc*) shmat(shmid, NULL, 0);
    	if(temp == (void*)-1) {
    		perror("shmat error: ");
    		return (void*)-1;
    	}

    	for(int i=0; i<MAX_STACK; i++){
    		(*(temp + i)).free = true;
    	}
    }

    stackdesc *new_stackdesc;
    new_stackdesc = (stackdesc*) shmat(shmid, NULL, 0);
    if(new_stackdesc == (void*)-1) {
    	perror("shmat error: ");
    	return (void*)-1;
    }

    int stackdescid;
    void *top;
    stackdescid = shmget(key, sizeof(element_size) * stack_size, IPC_CREAT | IPC_EXCL | 0777);
	if (stackdescid == -1) {
		stackdescid = shmget(key, sizeof(element_size) * stack_size, IPC_CREAT | 0777);
		top = shmat(stackdescid, NULL, 0);
		for(int i=0; i<MAX_STACK; i++) {
			if((*(new_stackdesc + i)).stackdescid == stackdescid) {
				P(semid);
				(*(new_stackdesc + i)).stack_top = top;
				V(semid);
				return new_stackdesc;
			}
 		}
	} else {	
 		top = shmat(stackdescid, NULL, 0);
 		for(int i=0; i<MAX_STACK; i++) {
			if((*(new_stackdesc + i)).free == true){
				P(semid);
				(*(new_stackdesc + i)).stack_top = top;
				(*(new_stackdesc + i)).stackKey = key;
				(*(new_stackdesc + i)).stackdescid = stackdescid;
				(*(new_stackdesc + i)).data_size = element_size;
				(*(new_stackdesc + i)).stack_size = stack_size;
				(*(new_stackdesc + i)).elements_no = 0;
				(*(new_stackdesc + i)).free = false;
				V(semid);
				return new_stackdesc;
			}
		}
		printf("\nNo stackdesc free!\n");
		shmdt(temp);
	    return (void*)-1;
	}
}

int shstackpush(stackdesc *p_desc, void *element_addr) {
	if((*(p_desc)).elements_no == (*(p_desc)).stack_size) {
		// printf("\nStack Overflow!\n");
		return -1;
	}

	void *target = (char *)(*(p_desc)).stack_top + (*(p_desc)).data_size * (*(p_desc)).elements_no;
	P(semid);
	memcpy(target, element_addr, (*(p_desc)).data_size);
	(*(p_desc)).elements_no++;
	V(semid);

	return 0;
}

void* shstackpop(stackdesc *p_desc, void* element_addr) {
	if((*(p_desc)).elements_no == 0) {
		// printf("\nStack Underflow!\n");
		return (void*)-1;
	}

	void *source = (char *)(*(p_desc)).stack_top + ((*(p_desc)).elements_no - 1) * (*(p_desc)).data_size;

	P(semid);
    memcpy(element_addr, source, (*(p_desc)).data_size);
    (*(p_desc)).elements_no--;
    V(semid);

    return element_addr;
}

void shstackrm(stackdesc *p_desc) {
	shmdt((*(p_desc)).stack_top);
	shmctl((*(p_desc)).stackdescid, IPC_RMID, NULL);
	if (owner) {
		shmdt(temp);
		shmctl(shmid, IPC_RMID, NULL);
	}
}

void* shstacktop(stackdesc *p_desc, void* element_addr) {
	if((*(p_desc)).elements_no == 0){
		printf("\nStack Empty!\n");
		return (void*)-1;
	}
	void *source = (char *)(*(p_desc)).stack_top + ((*(p_desc)).elements_no - 1) * (*(p_desc)).data_size;

	P(semid);
    memcpy(element_addr, source, (*(p_desc)).data_size);
    V(semid);
    return element_addr;
}

int shstacksize(stackdesc *p_desc) {
	return (*(p_desc)).elements_no;
}


int main() {
	key_t mykey = ftok("/home/saptarshi/Desktop/OS/assgn5/2.c", 3);

	stackdesc *temp = shstackget(mykey, sizeof(float), 2);
	float val = 1.1;
	float *ptr;
	
	if(shstackpush(temp, &val) == -1)
		printf("Stack Overflow!\n");
	
	val = 2.2;
	if(shstackpush(temp, &val) == -1)
		printf("Stack Overflow!\n");

	temp = shstackget(mykey, sizeof(float), 2);
	
	ptr = (float*)shstackpop(temp, &val);
	if(ptr == (void*)-1)
		printf("Stack Underflow!\n");
	else printf("%f\n", *ptr);
	
	ptr = (float*)shstackpop(temp, &val);
	if(ptr == (void*)-1)
		printf("Stack Underflow!\n");
	else printf("%f\n", *ptr);

	shstackrm(temp);

	return 0;
}