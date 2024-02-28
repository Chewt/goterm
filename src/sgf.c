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
#include "stack.h"

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
    unsigned int fileSize = 4096;
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

    // Get size of file
    fseek(f, 0, SEEK_END);
    int f_size = ftell(f);
    rewind(f);

    // Make buffer to store file contents
    char* sgf = malloc(f_size);

    // Read contents into buffer
    int i = 0;
    char c;
    while ((c = fgetc(f)) != EOF)
        sgf[i++] = c;

    fclose(f);
    return sgf;
}

char* FindTagEnd(char* src)
{
    char* c = src;
    while (*c != ']')
    {
        if (*c == '\0') // if we find a null char this isn't a tag
            return c;
        if (*c == '\\') // Skip sequence '\]'
            c++;
        c++;
    }
    return c;
}

/*  copy at most n chars of src to dst, up until the character ']' is found.
 *  Ignores sequence '\]'
 *
 *  Returns pointer to location in src right after ']'.
 */
char* CopyTagContents(char* dst, char* src, int n)
{
    char* c = FindTagEnd(src);
    char temp = *c;
    *c = '\0';
    strncpy(dst, src, n - 1);
    *c = temp;
    return c + 1;
}

// Return a pointer to the beginning of a label following a tag
char* FindNextLabel(char* src)
{
    char* c = src;
    do 
    {
        c = FindTagEnd(c);
        while ((*c == '\n') || (*c == ']'))
            c++;
    } while (*c == '[');
    return c;
}

// Return a pointer to the opening '[' of a tag
char* FindNextTag(char* src)
{
    if (!src)
        return src;
    char* c = src;
    while ((*c != '[') && (*c != '\0'))
        c++;
    return c;
}

void LoadSGF(Goban* goban, char* sgf)
{
    // Start from clean state
    GameInfo* gameInfo = GetGameInfo();
    ResetGoban(goban);
    NewTree(goban);
    char* token;
    char* save_ptr;
    token = strtok_r(sgf, ";", &save_ptr); // First token always '(' 

    // Get metadata
    token = strtok_r(NULL, ";", &save_ptr);
    char* label = token;
    char* label_end = FindNextTag(label);
    while (label[0] != '\0')
    {
        if (!strncmp(label, "PW", label_end - label))
        {
            bzero(gameInfo->whiteName, NAME_LENGTH);
            CopyTagContents(gameInfo->whiteName, label_end + 1, NAME_LENGTH);
        }
        else if (!strncmp(label, "PB", label_end - label))
        {
            bzero(gameInfo->blackName, NAME_LENGTH);
            CopyTagContents(gameInfo->blackName, label_end + 1, NAME_LENGTH);
        }
        else if (!strncmp(label, "KM", label_end - label))
        {
            char* tag_end = FindTagEnd(label_end);
            char temp = *tag_end;
            *tag_end = '\0';
            gameInfo->komi = strtof(label_end + 1, NULL);
            *tag_end = temp;
        }
        else if (!strncmp(label, "SZ", label_end - label))
        {
            char* tag_end = FindTagEnd(label_end);
            char temp = *tag_end;
            *tag_end = '\0';
            gameInfo->boardSize = strtod(label_end + 1, NULL);
            *tag_end = temp;
        }
        else if (!strncmp(label, "HA", label_end - label))
        {
            char* tag_end = FindTagEnd(label_end);
            char temp = *tag_end;
            *tag_end = '\0';
            SetHandicap(goban, strtod(label_end + 1, NULL));
            *tag_end = temp;
        }
        else if (!strncmp(label, "RE", label_end - label))
        {
            bzero(gameInfo->result, RESULT_LENGTH);
            CopyTagContents(gameInfo->result, label_end + 1, RESULT_LENGTH);
        }
        label = FindNextLabel(label_end);
        label_end = FindNextTag(label);
    } 

    // While reading tags it is possible for a comment to have a ; in it, so lets capture that
    int partialComment = 0;

    // Read Moves
    while ((token = strtok_r(NULL, ";", &save_ptr)) != NULL)
    {
        label = token;
        label_end = FindNextTag(label);

        // Resume copy if partial comment was found before
        if (partialComment)
        {
            label--;
            *label = ';';
            partialComment = 0;
            GameNode* node = GetViewedNode();
            char* tagEnd = FindTagEnd(label);
            CopyTagContents(node->comment + strlen(node->comment), label,
                            COMMENT_LENGTH - strlen(node->comment));
            *label = '\0';
            if (*tagEnd != ']')
            {
                partialComment = 1;
                continue;
            }
        }

        // This is to hard cut-off adding moves until I have a proper branch system
        int end_of_branch = 0;
        while (label[0] != '\0')
        {
            if (label[0] == ')')
            {
                end_of_branch = 1;
                break;
            }
            if (!strncmp(label, "W", label_end - label) ||
                !strncmp(label, "B", label_end - label))
            {
                Move m;
                m.color = (label[0] == 'B') ? 'b' : 'w';
                if ((label[1] == '[') && (label[2] == ']')) // Pass
                {
                    goban->lastmove.p.col = -1;
                    goban->lastmove.p.row = -1;
                    goban->lastmove.color = m.color;
                    goban->color = (goban->color == 'b') ? 'w' : 'b';
                    AddHistory(goban);
                }
                else // Real move
                {
                    m.p.col = label[2] - 'a';
                    m.p.row = label[3] - 'a';
                    ValidateMove(goban, m);
                }
            }
            else if (!strncmp(label, "C", label_end - label))
            {
                GameNode* node = GetViewedNode();
                char* tagEnd = FindTagEnd(label);
                if (*tagEnd != ']')
                    partialComment = 1;
                CopyTagContents(node->comment, label_end + 1, COMMENT_LENGTH);
            }
            label = FindNextLabel(label);
            label_end = FindNextTag(label);
        }
        if (end_of_branch)
            break;
    }
    WriteNotes("Loaded game from sgf...\n");
}
