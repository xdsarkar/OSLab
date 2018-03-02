#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include "sharedstack.h"

#define keyg(i) ftok("..", i)
#define keypub(i) ftok(".", i)

int main()
{
    instructions();
    pid_t pid = fork();

    if(pid == 0)
    {   
        intialize();  
        
        key_t key1 = keyg(5);
        key_t key3 = keyg(7);   
        int stack_id = shstackget(key1, sizeof(int), 10, IPC_CREAT|0777);
        int xyz = shstackget(key3, sizeof(int), 10, IPC_CREAT|0777);
        
        shstackpush(stack_id, 1);
        shstackpush(stack_id, 2);
        shstackpush(stack_id, 3);
        shstackpush(stack_id, 4);
        shstackpush(stack_id, 5);
        shstackpush(stack_id, 8);
        shstackpush(xyz,6);
        
        print_current_stack(stack_id);
        print_current_stack(xyz);
    }
    else
    {
        wait(NULL);
        
        intialize();
        
        key_t key2 = keyg(5);
        key_t key4 = keyg(7);     
        int stack_id = shstackget(key2, sizeof(int), 10, IPC_CREAT|0777);
        int xyz = shstackget(key4, sizeof(int), 10, IPC_CREAT|0777);

        shstackpop(stack_id);
        shstackpop(stack_id);
        shstackpop(stack_id);
        shstackpop(stack_id);
        shstackpop(xyz);
        
        shstackrm(stack_id);
        shstackrm(xyz);
    }
    return 0;
}