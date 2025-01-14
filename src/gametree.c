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

void NodeAddGoban(GameNode* node, Goban* goban)
{
    memcpy(&(node->goban), goban, sizeof(Goban));
}

GameNode* NewNode()
{
    GameNode* node = malloc(sizeof(struct GameNode));
    node->mainline_prev = NULL;
    node->mainline_next = NULL;
    node->n_alts = 0;
    node->comment[0] = '\0';
    memset(node->labels, ' ', 19 * 19);
    return node;
}

GameNode* AddMainline(GameNode* node, Goban* goban) 
{
    node->mainline_next = NewNode();
    NodeAddGoban(node->mainline_next, goban);
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

    node->alts[node->n_alts] = NewNode();
    NodeAddGoban(node->alts[node->n_alts], goban);
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
    root_node = NewNode();
    NodeAddGoban(root_node, goban);
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
    int i;
    GameNode* base = viewed_node;
    for (i = 0; i < n && (viewed_node->mainline_prev != NULL); ++i)
        viewed_node = viewed_node->mainline_prev;
    GameNode* toFree = viewed_node->mainline_next;
    n = i;

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
    int count = 1;
    GameNode* node = GetRootNode();
    while (node->mainline_next)
    {
        count++;
        node = node->mainline_next;
    }
    return count;
}

/* if Direction < 0  jump up (alts - 1)
 * if Direction >= 0 jump down (alts + 1)
 */
void JumpBranch(Goban* goban, int direction)
{
    int count = 0;

    // Find branch point
    GameNode* current = GetViewedNode();


    // Special case
    if (current->n_alts && direction >= 0)
    {
        view_index++;
        viewed_node = current->alts[0];
        memcpy(goban, &(viewed_node->goban), sizeof(Goban));
        return;
    }

    while (current->mainline_prev && !current->mainline_prev->n_alts)
    {
        current = current->mainline_prev;
        count++;
    }

    if (current->mainline_prev == NULL)
        return;

    int i;
    for (i = 0; i < current->mainline_prev->n_alts; ++i)
    {
        if (current->mainline_prev->alts[i] == current)
            break;
    }

    if (direction < 0) // Up
    {
        if (i == 0) // Up to mainline
            current = current->mainline_prev->mainline_next;
        else if (direction < 0) // Up
            current = current->mainline_prev->alts[i - 1];
    }
    else // Down
    {
        if (i < current->mainline_prev->n_alts - 1) 
            current = current->mainline_prev->alts[i + 1];
        else if (i == current->mainline_prev->n_alts)
            current = current->mainline_prev->alts[0];
    }

    for (i = 0; i < count && current->mainline_next; ++i)
        current = current->mainline_next;

    viewed_node = current;
    memcpy(goban, &(viewed_node->goban), sizeof(Goban));
}

int CountBranches(GameNode* base, int limit)
{
    GameNode* node = base;
    int count = 0;
    int i, j;
    for (i = 0; i < limit && node != NULL; ++i)
    {
        if (node->n_alts)
        {
            for (j = 0; j < node->n_alts; ++j)
                count += CountBranches(node->alts[j], limit - i - 1) + 1;
        }
        node = node->mainline_next;
    }
    return count;
}

int CountNodes(GameNode* base)
{
    GameNode* node = base;
    int count = 1;
    int i;
    while (node->mainline_next)
    {
        count++;
        if (node->n_alts)
        {
            for (i = 0; i < node->n_alts; ++i)
                count += CountNodes(node->alts[i]);
        }
        node = node->mainline_next;
    }
    return count;
}

void SetViewedNode(Goban* goban, GameNode* node)
{
    viewed_node = node;
    memcpy(goban, &(viewed_node->goban), sizeof(Goban));
}
