#include "gametree.h"
#include <stdlib.h>
#include <string.h>

GameNode* root_node = NULL;

GameNode* GetRootNode()
{
    return root_node;
}

GameNode* NewNode(Goban* goban)
{
    GameNode* root = malloc(sizeof(struct GameNode));
    memcpy(&(root->goban), goban, sizeof(Goban));
    root->mainline_prev = NULL;
    root->mainline_next = NULL;
    root->n_alts = 0;
    root->comment[0] = '\0';
    return root;
}

GameNode* AddMainline(GameNode* node, Goban* goban)
{
    node->mainline_next = NewNode(goban);
    node->mainline_next->mainline_prev = node;
    return node->mainline_next;
}

GameNode* RetrieveNode(int idx)
{
    GameNode* current = root_node;
    int i;
    for (i = 0; (i < idx) && (current->mainline_next != NULL); i++)
        current = current->mainline_next;
    return current;
}

void FreeTree(GameNode* root)
{
    int i;
    for (i = 0; i < root->n_alts; ++i)
        FreeTree(root->alts[i]);
    if (root->mainline_next != NULL)
        FreeTree(root->mainline_next);
    free(root);
    root = NULL;
}
