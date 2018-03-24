#include "sharedstack.h"
#include <sys/types.h>
#include <sys/ipc.h>

#define keyg(i) ftok("..", i)
#define keypub(i) ftok(".", i)

int main()
{
    instructions();
    
    intialize();  

    key_t key = keyg(5);
    int stack_id = shstackget(key, sizeof(int), 10, IPC_CREAT|0777);
    key_t key1 = keyg(6);
    int stack_x = shstackget(key1, sizeof(int), 10, IPC_CREAT|0777);
    shstackpush(stack_x, 3);
    shstackpop(stack_id);
    //shstackpush(stack_x, 3);
    shstackrm(stack_id);
    return 0;
}