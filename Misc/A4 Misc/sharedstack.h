#ifndef SHAREDSTACK_H_INCLUDED
#define SHAREDSTACK_H_INCLUDED

#define MAX_STACK 10
#include <stdio.h>			// standard input/output
#include <sys/ipc.h>        // shmget(), shmctl()
#include <sys/shm.h>        // shmget(), shmat(), shmdt(), shmctl()
#include <sys/types.h>      // shmat(), shmdt()
#include <stdlib.h>         // exit()
#include <stdbool.h>		// bool
#include <string.h>

typedef struct 
{
	key_t 	stackKey;			// key for the shared memory of a stack
	int 	stackdescid;		// shared stack memory segment id
	int 	data_size;			// data element size for this stack
	int 	stack_size;			// size for this stack
	void 	*stack_top;			// pointer to this stack
	int 	elements_no;		// present no of elements
	bool 	free;				// whether this descriptor is free for use
}stackdesc;

extern bool owner;
stackdesc *temp;
int shmid;

stackdesc* shstackget(key_t, int, int);
int shstackpush(stackdesc*, void*);
void* shstackpop(stackdesc*, void*);
void shstackrm(stackdesc*);
void* shstacktop(stackdesc*, void*);

#endif