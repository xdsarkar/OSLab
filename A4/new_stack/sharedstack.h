#ifndef SHAREDSTACK_H_INCLUDED
#define SHAREDSTACK_H_INCLUDED

#include<sys/types.h>
#include <stdio.h>  /* standard input/output */
#include <sys/ipc.h> /* shmget(), shmctl() */
#include <sys/shm.h> /* shmget(), shmat(), shmdt(), shmctl() */
#include <sys/types.h> /* shmat(), shmdt() */
#include <stdlib.h> /* exit() */
#include <stdbool.h> /* bool == true/false */
#include <sys/sem.h> /* semget(), semctl() */
#include <string.h> /* memcpy() */
#include <unistd.h> /* sleep() */

void instructions();
void intialize();
void shstackpush(int, int);
void shstackpop(int);
void shstackrm(int);
void print_current_stack(int);
int shstackget(key_t, int, int, int);

/* max number of stacks */
#define MAX_STACK 10

/* key generators */
#define keypub(i) ftok(".", i)
#define keyg(i) ftok("..", i)

/* int semop(int semid, struct sembuf *sops, size_t nsops); */
#define P(s) semop(s, &POP, 1);
#define V(s) semop(s, &VOP, 1);

/* defining structure sembuf for 2 operations Pop/Vop */
struct sembuf POP;
struct sembuf VOP;

key_t univ_key;
int global_shmid;
int semid;

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

#endif