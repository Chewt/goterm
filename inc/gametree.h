#ifndef GAMETREE_H
#define GAMETREE_H

#include "go.h"

#define COMMENT_LENGTH 2048
#define MAX_BRANCHES 10

struct GameNode
{
    Goban goban;
    struct GameNode* mainline_next;
    struct GameNode* mainline_prev;
    struct GameNode* alts[MAX_BRANCHES];
    int n_alts;
    char comment[COMMENT_LENGTH];
};
typedef struct GameNode GameNode;

GameNode* NewNode(Goban* goban);
GameNode* AddMainline(GameNode* node, Goban* goban);
GameNode* RetrieveNode(int idx);
int FreeTree(GameNode* root);
void NewTree(Goban* goban);

int GetHistorySize();
int GetViewIndex();
GameNode* GetRootNode();
GameNode* GetViewedNode();
GameNode* GetHistory(int i);
void AddHistory(Goban* goban);
void UndoHistory(Goban* goban, int n);
void ViewHistory(Goban* goban, int n);
void SlideHistory(Goban* goban, int n);

#endif
