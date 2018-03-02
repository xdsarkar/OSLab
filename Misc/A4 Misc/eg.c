#include <stdio.h> // standard input/output
#include <sys/ipc.h> // shmget(), shmctl()
#include <sys/shm.h> // shmget(), shmat(), shmdt(), shmctl()
#include <sys/types.h> // shmat(), shmdt()
#include <stdlib.h> // exit()
#include <stdbool.h> // bool
#include <string.h> // memcpy()
#include "sharedstack.h"

bool owner = false;
int main(int argc, char const *argv[])
{
	key_t mykey = ftok(".", 3);

	stackdesc *temp = shstackget(mykey, sizeof(float), 1);
	float val = 1.1;
	
	if(shstackpush(temp, &val) == -1)
		printf("Stack Overflow!\n");
	
	val = 2.2;
	if(shstackpush(temp, &val) == -1)
		printf("Stack Overflow!\n");
	
	shstackrm(temp);
	return 0;
}
