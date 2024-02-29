#include "stack.h"

void ClearPStack(PointStack* stack)
{
    stack->sp = 0;
}

void PushPStack(PointStack* stack, Point point)
{
    stack->data[stack->sp] = point;
    stack->sp++;
}

Point PopPStack(PointStack* stack)
{
    stack->sp--;
    return stack->data[stack->sp];
}

int PStackSize(PointStack* stack)
{
    return stack->sp;
}

void ClearNStack(NodeStack* stack)
{
    stack->sp = 0;
}

void PushNStack(NodeStack* stack, GameNode* node)
{
    stack->data[stack->sp] = node;
    stack->sp++;
}

GameNode* PopNStack(NodeStack* stack)
{
    stack->sp--;
    return stack->data[stack->sp];
}

int NStackSize(NodeStack* stack)
{
    return stack->sp;
}
