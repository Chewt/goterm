#ifndef STACK_H
#define STACK_H

#include "go.h"
#include "gametree.h"
#define STACK_SIZE 361

typedef struct
{
    Point data[STACK_SIZE];
    int sp;
} PointStack;

typedef struct
{
    GameNode* data[STACK_SIZE];
    int sp;
} NodeStack;

void ClearPStack(PointStack* stack);
Point PopPStack(PointStack* stack);
void PushPStack(PointStack* stack, Point point);
int PStackSize(PointStack* stack);

void ClearNStack(NodeStack* stack);
GameNode* PopNStack(NodeStack* stack);
void PushNStack(NodeStack* stack, GameNode* node);
int NStackSize(NodeStack* stack);
#endif
