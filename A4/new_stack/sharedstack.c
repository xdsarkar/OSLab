#include "sharedstack.h" /* personal creater header */

key_t univ_key;
int global_shmid;

/* for loops */
#define FOR(i,j) for(int i=0; i<j; i++)
#define FORB(i,j) for(int i=j; i>0; i--)

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
    union semun 
    {
        int              val;    /* Value for SETVAL */
        struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
        unsigned short  *array;  /* Array for GETALL, SETALL */
        struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
    } setvalArg;

    setvalArg.val = 1;

    key_t key_sem = keyg(10);
    int semid = semget(key_sem, 1,IPC_CREAT | 0777 | IPC_EXCL); /* int semget(key_t key, int nsems, int semflg); */
    if(semid == -1) 
    {
        semid = semget(key_sem, 1, IPC_CREAT | 0777);
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
            semctl(semid, id_stack, SETVAL, setvalArg);
            shmdt((void *)stack_ptr);
            return id_stack;
        }
    }
    shmdt((void *)stack_ptr);
    return -1; /* shstackget failure */
}

void shstackpush(int stack_id, int element)
{
    /* pop */
    POP.sem_num = stack_id;
    POP.sem_op = -1;
    POP.sem_flg = SEM_UNDO;

    /* vop */
    VOP.sem_num = stack_id;
    VOP.sem_op = 1;
    VOP.sem_flg = SEM_UNDO;

    stack_sh* stack_ptr = (stack_sh*)shmat(global_shmid, NULL, 0);
    P(semid);
    int elem_num = stack_ptr->shared_stack[stack_id].elem_num;
    int st_size = stack_ptr->shared_stack[stack_id].stack_size;
    if(elem_num == st_size) /* stack is full */
    {
        printf("Stack ID = %d >> Stack is full! Pop first, duh!\n", stack_id);
        V(semid);
    }
    else /* push operation can be done */
    {
        int *ptr = (int *)shmat(stack_ptr->shared_stack[stack_id].shmid, NULL, 0);
        int elem_num = stack_ptr->shared_stack[stack_id].elem_num;
        *(ptr + elem_num) = element;
        stack_ptr->shared_stack[stack_id].elem_num++;
        printf("Stack ID = %d >> Pushed element: %d\n", stack_id, element);
        V(semid);  
        shmdt((void *)ptr);
    }
    shmdt((void *)stack_ptr);
}

void shstackpop(int stack_id)
{
    /* pop */
    POP.sem_num = stack_id;
    POP.sem_op = -1;
    POP.sem_flg = SEM_UNDO;

    /* vop */
    VOP.sem_num = stack_id;
    VOP.sem_op = 1;
    VOP.sem_flg = SEM_UNDO;

    stack_sh* stack_ptr = (stack_sh*)shmat(global_shmid, NULL, 0);
    if(stack_ptr == (void *)(-1)) perror("shmat() failed: ");
    int elem_num = stack_ptr->shared_stack[stack_id].elem_num;
    
    P(semid);
    if(elem_num == 0) /* stack is empty */
    {
        printf("Stack ID = %d >> Stack is empty! You can't pop, anymore!\n", stack_id);
        V(semid);
    }
    else if(elem_num > 0) /* pop from stack can be done */
    {
        int *ptr = (int *)shmat(stack_ptr->shared_stack[stack_id].shmid, NULL, 0);
        int element = *(ptr + elem_num - 1);
        stack_ptr->shared_stack[stack_id].elem_num--;
        printf("Stack ID = %d >> Popped element: %d\n", stack_id, element);
        V(semid);
        shmdt((void *)ptr);
    }
    shmdt((void *)stack_ptr);
}

void shstackrm(int stack_id)
{
    /* pop */
    POP.sem_num = stack_id;
    POP.sem_op = -1;
    POP.sem_flg = SEM_UNDO;

    /* vop */
    VOP.sem_num = stack_id;
    VOP.sem_op = 1;
    VOP.sem_flg = SEM_UNDO;

    sleep(1); 
    /* Observation: When shstackpush() is called, it pushes fine and as well as prints,
    but if shstackpop() is immediately used prior to shstackrm(), 
    pop was done but it couldn't print. So introduction to sleep, "helps" */

    stack_sh* stack_ptr = (stack_sh*)shmat(global_shmid, NULL, 0);
    P(semid);
    if(shmctl(stack_ptr->shared_stack[stack_id].shmid, IPC_RMID, NULL) == 1) 
    {
        printf("(+) Stack Remove >> Unsuccessful!\n");
        V(semid);
    }
    else 
    {
        printf("(+) Stack Remove >> Sucessful! with Stack ID = %d\n", stack_id);
        V(semid);
    }
    
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
        if(shmctl(global_shmid, IPC_RMID, NULL) == 1) 
        {
            printf("(+) Shared Stack Remove >> Unsuccessful!\n");
            V(semid);
        }
        else
        {
            printf("(+) Shared Stack Remove >> Sucessful!\n");
            stack_ptr->initialize_stack = false;
            FOR(i, MAX_STACK) 
            {
                if(stack_ptr->shared_stack[i].free == false)
                {
                    V(semid);
                    shmctl(stack_ptr->shared_stack[i].shmid, IPC_RMID, NULL);
                    printf("(+) Stack ID = %d is also deleted as shared stack is not there!\n", i);
                }
            }
        }
    }
    else 
    {
        printf("Okay, as you wish! But don't forget to delete me! Else when you ipcs, you'll see me there!\n");
        V(semid);
    }
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