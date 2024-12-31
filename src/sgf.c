#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "sgf.h"
#include "display.h"
#include "gameinfo.h"
#include "gametree.h"
#include "go.h"

// Return size in bytes of all comments in game tree
unsigned int TreeCommentSize(GameNode* base)
{
    if (base == NULL)
        return 0;
    unsigned int count = 0;
    while (base)
    {
        count += strlen(base->comment);
        if (base->n_alts)
        {
            int i;
            for (i = 0; i < base->n_alts; ++i)
                count += TreeCommentSize(base->alts[i]);
        }
        base = base->mainline_next;
    }
    return count;
}

int WriteMoves(char* sgf, GameNode* base)
{
    unsigned int index = 0;
    while (base && !base->n_alts)
    {
        Move lm = base->goban.lastmove;
        if (lm.p.col != -1)
        {
            index += sprintf(sgf + index, ";%c[%c%c]",
                    lm.color - 32, 'a' + lm.p.col, 'a' + lm.p.row);
            if (base->comment[0] != '\0')
            {
                index += sprintf(sgf + index, "C[%s]", base->comment);
            }
        }
        base = base->mainline_next;
    }
    if (base)
    {
        Move lm = base->goban.lastmove;
        if (lm.p.col != -1)
        {
            index += sprintf(sgf + index, ";%c[%c%c]",
                    lm.color - 32, 'a' + lm.p.col, 'a' + lm.p.row);
            if (base->comment[0] != '\0')
            {
                index += sprintf(sgf + index, "C[%s]", base->comment);
            }
        }
        index += sprintf(sgf + index, "(");
        index += WriteMoves(sgf + index, base->mainline_next);
        int i;
        for (i = 0; base && i < base->n_alts; ++i)
        {
            index += sprintf(sgf + index, "(");
            index += WriteMoves(sgf + index, base->alts[i]);
        }
    }
    index += sprintf(sgf + index, ")");
    return index;
}

char* CreateSGF()
{
    GameInfo* gameInfo = GetGameInfo();
    if (GetHistorySize() <= 1)
        return NULL;
    time_t rawtime = time(NULL);
    struct tm* ptm = localtime(&rawtime);
    
    // Determine filesize
    GameNode* current = GetRootNode();
    unsigned int fileSize = 1024 + (CountNodes(current) * 9);
    fileSize += TreeCommentSize(current);

    char* sgf = malloc(fileSize);
    int index = 0;

#ifdef VERSION
    const char* version = VERSION;
#elif 
    const char* version = "debug";
#endif

    index = sprintf(sgf + index, "(;FF[4]GM[1]SZ[%d]AP[Goterm:%s]\n\n", 
            gameInfo->boardSize, version);
    index += sprintf(sgf + index, "PB[%s]\n", gameInfo->blackName);
    index += sprintf(sgf + index, "PW[%s]\n", gameInfo->whiteName);
    index += sprintf(sgf + index, "DT[");
    index += strftime(sgf + index, 11, "%F", ptm);
    index += sprintf(sgf + index, "]\n");
    index += sprintf(sgf + index, "KM[");
    index += sprintf(sgf + index, "%.1f", gameInfo->komi);
    index += sprintf(sgf + index, "]\n");
    index += sprintf(sgf + index, "HA[%d]\n", gameInfo->handicap);
    if (gameInfo->result[0] != '\0')
    {
        index += sprintf(sgf + index, "RE[");
        index += sprintf(sgf + index, "%s", gameInfo->result);
        index += sprintf(sgf + index, "]\n");
    }
    index += sprintf(sgf + index, "\n");

    index += WriteMoves(sgf + index, GetRootNode()->mainline_next);
    sgf[index] = '\0';
    return sgf;
}

char* ReadSGFFile(char* filename)
{
    // Open file
    FILE* f = fopen(filename, "r");
    if (f == NULL)
        return NULL;

    // Get size of file
    fseek(f, 0, SEEK_END);
    int f_size = ftell(f);
    rewind(f);

    // Make buffer to store file contents
    char* sgf = malloc(f_size + 1);

    // Read contents into buffer
    int i = 0;
    char c;
    while ((c = fgetc(f)) != EOF)
        sgf[i++] = c;
    sgf[i] = '\0';
    fclose(f);
    return sgf;
}

void JumpToPropEnd(char** sgf)
{
    // Jump to end of property
    while (**sgf != ']')
    {
        if (**sgf == '\\')
            (*sgf)++;
        (*sgf)++;
    }
}

/*  copy at most n chars of src to dst, up until the character ']' is found.
 *  Ignores sequence '\]'
 *
 */
void CopyStringPropContents(char* dst, char** src, int n)
{
    int idx = 0;
    while (**src && **src != ']' && idx < n)
    {
        if (**src == '\\')
            (*src)++;
        dst[idx++] = **src;
        (*src)++;
    }
    JumpToPropEnd(src);
}


/*
 * # SGF standard notes
 * --------------------
 * The following is the exact specification as taken from here:
 * https://www.red-bean.com/sgf/sgf4.html
 *
 * ## Notation
 * -----------
 *  "..." : terminal symbols
 *  [...] : option: occurs at most once
 *  {...} : repetition: any number of times, including zero
 *  (...) : grouping
 *    |   : exclusive or
 * italics: parameter explained at some other place
 *
 * ## Definitions
 * --------------
 *  Collection = GameTree { GameTree }
 *  GameTree   = "(" Sequence { GameTree } ")"
 *  Sequence   = Node { Node }
 *  Node       = ";" { Property }
 *  Property   = PropIdent PropValue { PropValue }
 *  PropIdent  = UcLetter { UcLetter }
 *  PropValue  = "[" CValueType "]"
 *  CValueType = (ValueType | Compose)
 *  ValueType  = (None | Number | Real | Double | Color | SimpleText |
 *                Text | Point  | Move | Stone)
 *
 */

void LoadProperty(GameNode* node, char** sgf)
{
    GameInfo* gameInfo = GetGameInfo();
    // Get property identifier string
    char prop_ident[3] = {0};
    prop_ident[0] = **sgf;
    (*sgf)++;
    if (**sgf >= 'A' && **sgf <= 'Z')
        prop_ident[1] = **sgf;

    // Go to next property value
    while (**sgf != '[')
        (*sgf)++;
    (*sgf)++;

    printf("found property %s\n", prop_ident);
    fflush(stdout);

    // Based on identifier, run appropriate property value processor function
    // "B" or "W" is a move
    if (!strncmp(prop_ident, "W", 2) || !strncmp(prop_ident, "B", 2))
    {
        Goban* goban = &(node->goban);
        Move m;
        m.color = (prop_ident[0] == 'B') ? 'b' : 'w';
        if (**sgf == ']') // Pass
        {
            printf("move was pass\n");
            goban->lastmove.p.col = -1;
            goban->lastmove.p.row = -1;
            goban->lastmove.color = m.color;
            goban->color = (goban->color == 'b') ? 'w' : 'b';
            AddHistory(goban);
        }
        else // Real move
        {
            printf("move was %c%c\n", (*sgf)[0], (*sgf)[1]);
            m.p.col = (*sgf)[0] - 'a';
            m.p.row = (*sgf)[1] - 'a';
            AddMove(goban, m);
        }
        JumpToPropEnd(sgf);
    }
    // "C" is a comment
    else if (!strncmp(prop_ident, "C", 2))
    {
        bzero(node->comment, COMMENT_LENGTH);
        CopyStringPropContents(node->comment, sgf, COMMENT_LENGTH);
    }
    // "PW" is white players name
    else if (!strncmp(prop_ident, "PW", 2))
    {
        bzero(gameInfo->whiteName, NAME_LENGTH);
        CopyStringPropContents(gameInfo->whiteName, sgf, NAME_LENGTH);
    }
    // "PB" is black players name
    else if (!strncmp(prop_ident, "PB", 2))
    {
        bzero(gameInfo->blackName, NAME_LENGTH);
        CopyStringPropContents(gameInfo->blackName, sgf, NAME_LENGTH);
    }
    // "WR" is white rank
    else if (!strncmp(prop_ident, "WR", 2))
    {
        bzero(gameInfo->whiteRank, RANK_LENGTH);
        CopyStringPropContents(gameInfo->whiteRank, sgf, RANK_LENGTH);
    }
    // "BR" is black rank
    else if (!strncmp(prop_ident, "BR", 2))
    {
        bzero(gameInfo->blackRank, RANK_LENGTH);
        CopyStringPropContents(gameInfo->blackRank, sgf, RANK_LENGTH);
    }
    // "KM" is komi
    else if (!strncmp(prop_ident, "KM", 2))
    {
        char* prop_end = *sgf;
        while (*prop_end != ']')
            prop_end++;
        char temp = *prop_end;
        *prop_end = '\0';
        gameInfo->komi = strtof(*sgf , NULL);
        *prop_end = temp;
        JumpToPropEnd(sgf);
    }
    else if (!strncmp(prop_ident, "SZ", 2))
    {
        char* prop_end = *sgf;
        while (*prop_end != ']')
            prop_end++;
        char temp = *prop_end;
        *prop_end = '\0';
        gameInfo->boardSize = strtod(*sgf , NULL);
        *prop_end = temp;
        JumpToPropEnd(sgf);
    }
    else if (!strncmp(prop_ident, "HA", 2))
    {
        char* prop_end = *sgf;
        while (*prop_end != ']')
            prop_end++;
        char temp = *prop_end;
        *prop_end = '\0';
        SetHandicap(&node->goban, strtod(*sgf, NULL));
        *prop_end = temp;
        JumpToPropEnd(sgf);
    }
    else if (!strncmp(prop_ident, "RE", 2))
    {
        bzero(gameInfo->result, RESULT_LENGTH);
        CopyStringPropContents(gameInfo->result, sgf, RESULT_LENGTH);
    }
    else
    {
        JumpToPropEnd(sgf);
        while ((*sgf)[1] == '[')
        {
            (*sgf) += 2;
            JumpToPropEnd(sgf);
        }
    }
}

void LoadGameNode(GameNode* node, char** sgf)
{
    while (**sgf && **sgf != ';' && **sgf != ')' && **sgf != '(')
    {
        // apply each property seen to the passed node
        if (**sgf >= 'A' && **sgf <= 'Z')
        {
            LoadProperty(node, sgf);
        }
        else
            (*sgf)++;
    }
}

void LoadGameTree(GameNode* root, char** sgf)
{
    GameNode* current_node = root;
    while (**sgf && **sgf != ')')
    {
        // Found new gametree
        if (**sgf == '(')
        {
            printf("Found new gametree\n");
            (*sgf)++;
            GameNode* new_branch;
            if (current_node->mainline_next)
            {
                new_branch = NewNode();
                NodeAddGoban(new_branch, &current_node->goban);
                new_branch->mainline_prev = current_node;
                current_node->alts[current_node->n_alts++] = new_branch;
            }
            else
                new_branch = current_node;

            LoadGameTree(new_branch, sgf);
        }
        // Found node
        else if (**sgf == ';')
        {
            printf("Found node!\n");
            fflush(stdout);
            (*sgf)++;
            LoadGameNode(current_node, sgf);
            current_node->mainline_next = NewNode();
            NodeAddGoban(current_node->mainline_next, &current_node->goban);
            current_node->mainline_next->mainline_prev = current_node;
            current_node = current_node->mainline_next;
        }
        else
            (*sgf)++;
        // Finishing gametree
         if (**sgf == ')')
            printf("finishing game tree\n");
    }
    (*sgf)++;
}

void LoadSGF(Goban* goban, char* sgf)
{
    printf("loading sgf\n");
    // Start from clean state
    ResetGoban(goban);

    printf("I'm here now\n");
    
    // First char is always "(", so lets skip that
    sgf++;

    // Load the game tree
    printf("Loading game tree!\n");
    fflush(stdout);
    LoadGameTree(GetRootNode(), &sgf);

    // Set view to first node
    ViewHistory(goban, 0);
    WriteNotes("Loaded game from sgf...\n");
}
