#include "stack.h"
#include "stdlib.h"

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

void ClearNQueue(NodeQueue* queue)
{
    queue->front = 0;
    queue->back = 0;
    queue->count = 0;
}

void PushNQueue(NodeQueue* queue, GameNode* node)
{
    if (queue->count == STACK_SIZE)
        return;
    queue->data[queue->back++] = node;
    queue->count++;
}

GameNode* PopNQueue(NodeQueue* queue)
{
    if (queue->count == 0)
        return NULL;
    GameNode* node = queue->data[queue->front++];
    queue->count--;
    return node;
}

int NQueueSize(NodeQueue* queue)
{
    return queue->count;
}
