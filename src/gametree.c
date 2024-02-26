#include "gametree.h"
#include "display.h"
#include "go.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

GameNode* root_node = NULL;
GameNode* viewed_node = NULL;
int view_index = 0;
int node_counter = 0;

GameNode* GetRootNode()
{
    return root_node;
}

GameNode* GetViewedNode()
{
    return viewed_node;
}

GameNode* NewNode(Goban* goban)
{
    GameNode* node = malloc(sizeof(struct GameNode));
    memcpy(&(node->goban), goban, sizeof(Goban));
    node->mainline_prev = NULL;
    node->mainline_next = NULL;
    node->n_alts = 0;
    node->comment[0] = '\0';
    return node;
}

GameNode* AddMainline(GameNode* node, Goban* goban)
{
    node->mainline_next = NewNode(goban);
    node->mainline_next->mainline_prev = node;
    return node->mainline_next;
}

GameNode* AddVariation(GameNode* node, Goban* goban)
{
    int i;
    GameNode* existing_node = NULL;
    for (i = 0; i < node->n_alts; ++i)
    {
        if (IsEqual(goban, &(node->alts[i]->goban)))
        {
            existing_node = node->alts[i];
            break;
        }
    }
    if (IsEqual(goban, &(node->mainline_next->goban)))
        existing_node = node->mainline_next;
    if (existing_node != NULL)
        return existing_node;

    if (node->n_alts == MAX_BRANCHES)
        return NULL;

    node->alts[node->n_alts] = NewNode(goban);
    node->alts[node->n_alts]->mainline_prev = node;
    node->n_alts++;
    return node->alts[node->n_alts - 1];
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

int FreeTree(GameNode* root)
{
    int i;
    int nodes_removed = 0;
    for (i = 0; i < root->n_alts; ++i)
        nodes_removed += FreeTree(root->alts[i]);
    if (root->mainline_next != NULL)
        nodes_removed += FreeTree(root->mainline_next);
    free(root);
    root = NULL;
    return nodes_removed + 1;
}

int AddHistory(Goban* goban)
{
    if (viewed_node->mainline_next != NULL)
    {
        GameNode* new_node = AddVariation(viewed_node, goban); 
        if (new_node == NULL)
        {

            AppendNotes("Too many branches\n");
            return 0;
        }
        viewed_node = new_node;
    }
    else
    {
        viewed_node = AddMainline(viewed_node, goban);
        node_counter++;
    }
    view_index++;
    return 1;
}

void UndoHistory(Goban* goban, int n)
{
    if (node_counter - n > 0)
    {
        int i;
        GameNode* base = viewed_node;
        for (i = 0; i < n && (viewed_node->mainline_prev != NULL); ++i)
            viewed_node = viewed_node->mainline_prev;
        GameNode* toFree = viewed_node->mainline_next;

        for (i = 0; i < viewed_node->n_alts; ++i)
        {
            if (viewed_node->alts[i] == base)
            {
                int j;
                for (j = i + 1; j < viewed_node->n_alts; ++j)
                    viewed_node->alts[j - 1] = viewed_node->alts[j];
                viewed_node->alts[viewed_node->n_alts - 1] = NULL;
                viewed_node->n_alts--;
                toFree = base;
                break;
            }
        }
        node_counter -= FreeTree(toFree);
        if (toFree == viewed_node->mainline_next)
            viewed_node->mainline_next = NULL;
        memcpy(goban, &(viewed_node->goban), sizeof(Goban));
        view_index -= n;
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

    for (i = 0; i < n && viewed_node->mainline_next != NULL; ++i)
        viewed_node = viewed_node->mainline_next;

    memcpy(goban, &(viewed_node->goban), sizeof(Goban));
    view_index = n;
}

void SlideHistory(Goban* goban, int n)
{
    int i;
    if (n >= 0)
    {
        for (i = 0; i < n; ++i)
        {
            if (viewed_node->mainline_next == NULL)
                break;
            viewed_node = viewed_node->mainline_next;
        }
        view_index += i;
    }
    else if (n < 0)
    {
        for (i = 0; i < -n; ++i)
        {
            if (viewed_node->mainline_prev == NULL)
                break;
            viewed_node = viewed_node->mainline_prev;
        }
        view_index -= i;
    }
    memcpy(goban, &(viewed_node->goban), sizeof(Goban));
}

GameNode* GetHistory(int idx)
{
    GameNode* n = GetRootNode();
    int i;
    for (i = 0; i < idx; ++i)
        n = n->mainline_next;
    return n;
}

int GetViewIndex()
{
    return view_index;
}
int GetHistorySize()
{
    return node_counter;
}
