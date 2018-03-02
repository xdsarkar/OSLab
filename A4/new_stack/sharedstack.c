#include <stdio.h>  /* standard input/output */
#include <sys/ipc.h> /* shmget(), shmctl() */
#include <sys/shm.h> /* shmget(), shmat(), shmdt(), shmctl() */
#include <sys/types.h> /* shmat(), shmdt() */
#include <stdlib.h> /* exit() */
#include <stdbool.h> /* bool == true/false */
#include <sys/sem.h> /* semget(), semctl() */
#include <string.h> /* memcpy() */
#include <unistd.h> /* sleep() */
#include "sharedstack.h" /* personal creater header */

key_t univ_key;
int global_shmid;
/* max number of stacks */
#define MAX_STACK 10

/* for loops */
#define FOR(i,j) for(int i=0; i<j; i++)
#define FORB(i,j) for(int i=j; i>0; i--)

/* key generators */
#define keyg(i) ftok("..", i)
#define keypub(i) ftok(".", i)


// /* int semop(int semid, struct sembuf *sops, size_t nsops); */
// #define P(s) semop(s, &POP, 1);
// #define V(s) semop(s, &VOP, 1);

// /* defining structure sembuf for 2 operations Pop/Vop */
// struct sembuf POP;
// struct sembuf VOP;

// union semun 
// {
//     int              val;    /* Value for SETVAL */
//     struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
//     unsigned short  *array;  /* Array for GETALL, SETALL */
//     struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
// } setvalArg;

// union semun setvalArg = {.val = 1};

// /* pop */
// struct sembuf POP = {.sem_num = 0, .sem_op = -1, .sem_flg = SEM_UNDO};
// /* vop */
// struct sembuf VOP = {.sem_num = 0, .sem_op = 1, .sem_flg = SEM_UNDO};


// key_t key1, key2, key3;

// key1 = keyg(20);
// key2 = keyg(30);
// key3 = keyg(40);

// int semid1 = semget(key1,1,IPC_CREAT|0777|IPC_EXCL); /* int semget(key_t key, int nsems, int semflg); */
// if(semid1 == -1) 
// {
//     semid1 = semget(key1,1,IPC_CREAT|0777);
//     if(semid1 == -1) 
//     {
//         perror("semget() failed");
//         exit(1);
//     }
// } 
// else 
// {
//     int status1 = semctl(semid1,0,SETVAL,setvalArg);
//     if(status1 == -1) 
//     {
//         perror("semctl() failed");
//         exit(1);
//     }
// }

// int semid2 = semget(key2,1,IPC_CREAT|0777|IPC_EXCL); /* int semget(key_t key, int nsems, int semflg); */
// if(semid2 == -1) 
// {
//     semid2 = semget(key2,1,IPC_CREAT|0777);
//     if(semid2 == -1) 
//     {
//         perror("semget() failed");
//         exit(1);
//     }
// } 
// else 
// {
//     int status2 = semctl(semid2,0,SETVAL,setvalArg);
//     if(status2 == -1) 
//     {
//         perror("semctl() failed");
//         exit(1);
//     }
// }

// int semid3 = semget(key3,1,IPC_CREAT|0777|IPC_EXCL); /* int semget(key_t key, int nsems, int semflg); */
// if(semid3 == -1) 
// {
//     semid3 = semget(key3,1,IPC_CREAT|0777);
//     if(semid3 == -1) 
//     {
//         perror("semget() failed");
//         exit(1);
//     }
// } 
// else 
// {
//     int status3 = semctl(semid3,0,SETVAL,setvalArg);
//     if(status3 == -1) 
//     {
//         perror("semctl() failed");
//         exit(1);
//     }
// }

typedef struct
{
    key_t stackKey; /* key to the shared memory segment of the stack */
    int shmid; /* shmid of the shared memory segment allocated to the stack */
    int data_size; /* data element size of the stack: sizeof(int), sizeof(float), etc. */
    int stack_size; /* stack size: total size of stack */
    int elem_num; /* number of elements currently pushed in the stack */
    bool free; /* whether this descriptor is free for use: false/ true */
} stack_desc;

typedef struct
{
    stack_desc shared_stack[MAX_STACK];
    bool initialize_stack;
} stack_sh;

void intialize()
{   
    univ_key = keypub(3);
    global_shmid = shmget(univ_key, sizeof(stack_sh), IPC_CREAT | 0777);
    stack_sh* stack_ptr = (stack_sh*)shmat(global_shmid, NULL, 0);
    
    if(stack_ptr->initialize_stack == false)
    {
        stack_ptr->initialize_stack = true;
        FOR(i, MAX_STACK) stack_ptr->shared_stack[i].free = true;
    }
    shmdt((void *)stack_ptr);
}

int shstackget(key_t key, int element_size, int stack_size, int shm_stack_flg) 
{
    stack_sh* stack_ptr = (stack_sh*)shmat(global_shmid, NULL, 0);

    FOR(id_stack, MAX_STACK)
    {
        if(stack_ptr->shared_stack[id_stack].stackKey == key && stack_ptr->shared_stack[id_stack].free == false) /* if key == stackKey, stack descriptor is not free */
        {
            int id_exist = id_stack;
            shmdt((void *)stack_ptr);
            return id_exist; /* returns the id of the existing stack for given key */
        }

        if(stack_ptr->shared_stack[id_stack].free == true) /* if the stack descriptor is free */
        {
            int shmid = shmget(key, element_size * stack_size, shm_stack_flg);
            if(shmid == -1)
            {
                perror("shmget() failed: ");
                shmdt((void *)stack_ptr);
                return -1; /* return -1 if shmget() == -1 */
            }
            stack_ptr->shared_stack[id_stack].stackKey = key; /* key given */
            stack_ptr->shared_stack[id_stack].shmid = shmid; /* shmid given */
            stack_ptr->shared_stack[id_stack].data_size = element_size; /* elements size */
            stack_ptr->shared_stack[id_stack].stack_size = stack_size; /* initialise user defined stack size */
            stack_ptr->shared_stack[id_stack].elem_num = 0; /* stack is empty */
            stack_ptr->shared_stack[id_stack].free = false; /* stack is not free */
            shmdt((void *)stack_ptr);
            return id_stack;
        }
    }
    shmdt((void *)stack_ptr);
    return -1; /* shstackget failure */
}

void shstackpush(int stack_id, int element)
{
    stack_sh* stack_ptr = (stack_sh*)shmat(global_shmid, NULL, 0);
    int elem_num = stack_ptr->shared_stack[stack_id].elem_num;
    int st_size = stack_ptr->shared_stack[stack_id].stack_size; 
    if(elem_num == st_size) /* stack is full */
    {
        printf("Stack ID = %d >> Stack is full! Pop first, duh!\n", stack_id);
    }
    else /* push operation can be done */
    {
        int *ptr = (int *)shmat(stack_ptr->shared_stack[stack_id].shmid, NULL, 0);
        int elem_num = stack_ptr->shared_stack[stack_id].elem_num;
        *(ptr + elem_num) = element;
        stack_ptr->shared_stack[stack_id].elem_num++;
        printf("Stack ID = %d >> Pushed element: %d\n", stack_id, element);   
        shmdt((void *)ptr);
    }
    shmdt((void *)stack_ptr);
}

void shstackpop(int stack_id)
{
    stack_sh* stack_ptr = (stack_sh*)shmat(global_shmid, NULL, 0);
    if(stack_ptr == (void *)(-1)) perror("shmat() failed: ");
    
    int elem_num = stack_ptr->shared_stack[stack_id].elem_num;
    
    if(elem_num == 0) /* stack is empty */
    {
        printf("Stack ID = %d >> Stack is empty! You can't pop, anymore!\n", stack_id);
    }
    else if(elem_num > 0) /* pop from stack can be done */
    {
        int *ptr = (int *)shmat(stack_ptr->shared_stack[stack_id].shmid, NULL, 0);
        int element = *(ptr + elem_num - 1);
        stack_ptr->shared_stack[stack_id].elem_num--;
        printf("Stack ID = %d >> Popped element: %d\n", stack_id, element);
        shmdt((void *)ptr);
    }
    shmdt((void *)stack_ptr);
}

void shstackrm(int stack_id)
{
    sleep(1); /* Observation: When shstackpush() is called, it pushes fine and as well as prints,
    but if shstackpop() is immediately used prior to shstackrm(), 
    pop was done but it couldn't print. So introduction to sleep, "helps" */

    stack_sh* stack_ptr = (stack_sh*)shmat(global_shmid, NULL, 0);
    if(shmctl(stack_ptr->shared_stack[stack_id].shmid, IPC_RMID, NULL) == 1) printf("(+) Stack Remove >> Unsuccessful!\n");
    else printf("(+) Stack Remove >> Sucessful! with Stack ID = %d\n", stack_id);
    
    stack_ptr->shared_stack[stack_id].stackKey = -1;
    stack_ptr->shared_stack[stack_id].shmid = -1;
    stack_ptr->shared_stack[stack_id].data_size = 0;
    stack_ptr->shared_stack[stack_id].stack_size = 0;
    stack_ptr->shared_stack[stack_id].elem_num = 0;
    stack_ptr->shared_stack[stack_id].free = true; /* free to use */

    printf("Do you want to deleted the shared stack? (0: Yes, 1: No) -> ");
    fflush(stdout);
    int in; scanf("%d",&in);
    if(in == 0)
    {
        if(shmctl(global_shmid, IPC_RMID, NULL) == 1) printf("(+) Shared Stack Remove >> Unsuccessful!\n");
        else
        {
            printf("(+) Shared Stack Remove >> Sucessful!\n");
            stack_ptr->initialize_stack = false;
            FOR(i, MAX_STACK) 
            {
                if(stack_ptr->shared_stack[i].free == false)
                {
                    shmctl(stack_ptr->shared_stack[i].shmid, IPC_RMID, NULL);
                    printf("(+) Stack ID = %d is also deleted as shared stack is not there!\n", i);
                }
            }
        }  
    }
    else printf("Okay, as you wish! But don't forget to delete me! Else when you ipcs, you'll see me there!\n");
    shmdt((void *)stack_ptr);
}

void print_current_stack(int stack_id)
{
    stack_sh* stack_ptr = (stack_sh*)shmat(global_shmid, NULL, 0);
    
    int element, elem_num, st_size;
    int *ptr = (int *)shmat(stack_ptr->shared_stack[stack_id].shmid, NULL, 0);
    
    elem_num = stack_ptr->shared_stack[stack_id].elem_num; /* elements present */
    st_size = stack_ptr->shared_stack[stack_id].stack_size; /* stack size */

    printf("\nxxxxxxxxxxxxxx ELEMENTS PUSHED STACK ID = %d xxxxxxxxxxxxx\n\n", stack_id);
    FORB(i,st_size)
    {
        if(i <= elem_num)
        {
            element = *(ptr + i - 1);
            printf("==========================  %d  ==========================\n", element);
        }
        else 
        {
            printf("========================  EMPTY  ========================\n");
        }
    }
    printf("\nxxxxxxxxxxxxxxxxxxxxxx END OF STACK xxxxxxxxxxxxxxxxxxxxx\n\n");

    shmdt((void *)ptr); 
    shmdt((void *)stack_ptr);
}

void instructions()
{
    printf("\n----------------------------------------------------------------\n");
    printf("+ initialise(): initialize first\n");
    printf("+ shstackget(key, sizeof(int), stack_size, flags): gives you the respective stack_id\n");
    printf("+ shstackpush(stack_id, element): pushes the element to the respective stack\n");
    printf("+ shstackpop(stack_id): pops the element from the respective stack\n");
    printf("+ shstackrm(stack_id): removes the respective stack\n");
    printf("+ print_current_stack(stack_id): prints the full stack\n");
    printf("----------------------------------------------------------------\n\n");
}