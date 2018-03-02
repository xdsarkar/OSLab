#include "sharedstack.h"

stackdesc* shstackget(key_t key, int element_size, int stack_size) {
	key_t mykey;
	
    //key_t ftok(const char *pathname, int proj_id);
    mykey = ftok(".", 1);

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
				(*(new_stackdesc + i)).stack_top = top;
				return new_stackdesc;
			}
 		}
	} else {	
 		top = shmat(stackdescid, NULL, 0);
 		for(int i=0; i<MAX_STACK; i++) {
			if((*(new_stackdesc + i)).free == true){
				(*(new_stackdesc + i)).stack_top = top;
				(*(new_stackdesc + i)).stackKey = key;
				(*(new_stackdesc + i)).stackdescid = stackdescid;
				(*(new_stackdesc + i)).data_size = element_size;
				(*(new_stackdesc + i)).stack_size = stack_size;
				(*(new_stackdesc + i)).elements_no = 0;
				(*(new_stackdesc + i)).free = false;
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
	memcpy(target, element_addr, (*(p_desc)).data_size);
	(*(p_desc)).elements_no++;

	return 0;
}

void* shstackpop(stackdesc *p_desc, void* element_addr) {
	if((*(p_desc)).elements_no == 0) {
		// printf("\nStack Underflow!\n");
		return (void*)-1;
	}

	void *source = (char *)(*(p_desc)).stack_top + ((*(p_desc)).elements_no - 1) * (*(p_desc)).data_size;

    memcpy(element_addr, source, (*(p_desc)).data_size);
    (*(p_desc)).elements_no--;
    return element_addr;
}

void shstackrm(stackdesc *p_desc) {
	shmdt((*(p_desc)).stack_top);
	shmdt(temp);
	
	if (owner) {
		shmctl((*(p_desc)).stackdescid, IPC_RMID, NULL);
		shmctl(shmid, IPC_RMID, NULL);
	}
}

void* shstacktop(stackdesc *p_desc, void* element_addr) {
	if((*(p_desc)).elements_no == 0){
		printf("\nStack Empty!\n");
		return (void*)-1;
	}
	void *source = (char *)(*(p_desc)).stack_top + ((*(p_desc)).elements_no - 1) * (*(p_desc)).data_size;

    memcpy(element_addr, source, (*(p_desc)).data_size);
    return element_addr;
}

int shstacksize(stackdesc *p_desc) {
	return (*(p_desc)).elements_no;
}
