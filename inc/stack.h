#ifndef STACK_H
#define STACK_H

#include "go.h"
#define STACK_SIZE 361

typedef struct
{
    Point data[STACK_SIZE];
    int sp;
} Stack;

void ClearStack(Stack* stack);
Point PopStack(Stack* stack);
void PushStack(Stack* stack, Point point);
int StackSize(Stack* stack);

#endif
