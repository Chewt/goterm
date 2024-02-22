#include "gametree.h"
#include <stdlib.h>
#include <string.h>

GameNode* root_node = NULL;
GameNode* viewed_node = NULL;
int view_index = 0;
int node_counter = 0;

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

void NewTree(Goban* goban)
{
    if (root_node != NULL)
        FreeTree(root_node);
    root_node = NewNode(goban);
    viewed_node = root_node;
    node_counter = 1;
    view_index = 0;
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

void AddHistory(Goban* goban)
{
    viewed_node = AddMainline(viewed_node, goban);
    view_index++;
    node_counter++;
}

void UndoHistory(Goban* goban, int n)
{
    if (node_counter - n > 0)
    {
        node_counter -= n;
        int i;
        for (i = 0; i < n; ++i)
            viewed_node = viewed_node->mainline_prev;
        memcpy(goban, &(viewed_node->goban), sizeof(Goban));
        FreeTree(viewed_node->mainline_next);
        viewed_node->mainline_next = NULL;
    }
}

void ViewHistory(Goban* goban, int n)
{
    if (GetHistorySize() == 1)
        return;
    if (n >= GetHistorySize())
        n = GetHistorySize() - 1;
    if (n < 0)
        n = 0;
    int i;
    viewed_node = GetRootNode();
    for (i = 0; i < n; ++i)
    {
        if (viewed_node->mainline_next == NULL)
            break;
        viewed_node = viewed_node->mainline_next;
    }
    memcpy(goban, &(viewed_node->goban), sizeof(Goban));
    view_index = n;
}

Goban* GetHistory(int idx)
{
    GameNode* n = GetRootNode();
    int i;
    for (i = 0; i < idx; ++i)
        n = n->mainline_next;
    return &(n->goban);
}

int GetViewIndex()
{
    return view_index;
}
int GetHistorySize()
{
    return node_counter;
}
