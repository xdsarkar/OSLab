#ifndef SHAREDSTACK_H_INCLUDED
#define SHAREDSTACK_H_INCLUDED

#include<sys/types.h>

void instructions();
void intialize();
void shstackpush(int, int);
void shstackpop(int);
void shstackrm(int);
void print_current_stack(int);
int shstackget(key_t, int, int, int);

#endif