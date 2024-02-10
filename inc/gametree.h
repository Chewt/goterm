#ifndef GAMETREE_H
#define GAMETREE_H

#include "go.h"


struct GameNode
{
    Goban goban;
    struct GameNode* mainline_next;
    struct GameNode* mainline_prev;
    struct GameNode* alts[10];
    int n_alts;
    char comment[2048];
};
typedef struct GameNode GameNode;

GameNode* NewNode(Goban* goban);
GameNode* AddMainline(GameNode* node, Goban* goban);
GameNode* RetrieveNode(int idx);
void FreeTree(GameNode* root);

#endif
