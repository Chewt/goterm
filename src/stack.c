#include <string.h>
#include "stack.h"

void ClearStack(Stack* stack)
{
    stack->sp = 0;
}

void PushStack(Stack* stack, Point point)
{
    stack->data[stack->sp] = point;
    stack->sp++;
}

Point PopStack(Stack* stack)
{
    stack->sp--;
    return stack->data[stack->sp];
}

int StackSize(Stack* stack)
{
    return stack->sp;
}
