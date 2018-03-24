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

    shstackpush(stack_id, 1);
    shstackpush(stack_id, 2);
    shstackpush(stack_id, 3);
    shstackpush(stack_id, 4);
    shstackpush(stack_id, 5);
    shstackpush(stack_id, 8);
    print_current_stack(stack_id);
    return 0;
}