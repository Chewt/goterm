#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "sgf.h"
#include "go.h"

char* CreateSGF()
{
    if (HistorySize() <= 1)
        return NULL;
    time_t rawtime = time(NULL);
    struct tm* ptm = localtime(&rawtime);

    char* sgf = malloc(4096);
    int index = 0;
    
    Goban* current = GetHistory(0);
    Goban* last = GetHistory(HistorySize() - 1);

#ifdef VERSION
    const char* version = VERSION;
#elif 
    const char* version = "debug";
#endif

    index = sprintf(sgf + index, "(;FF[4]GM[1]SZ[%d]AP[Goterm:%s]\n\n", 
            last->size, version);
    index += sprintf(sgf + index, "PB[%s]\n", last->blackname);
    index += sprintf(sgf + index, "PW[%s]\n", last->whitename);
    index += sprintf(sgf + index, "DT[");
    index += strftime(sgf + index, 11, "%F", ptm);
    index += sprintf(sgf + index, "]\n");
    index += sprintf(sgf + index, "KM[");
    index += sprintf(sgf + index, "%.1f", last->komi);
    index += sprintf(sgf + index, "]\n");
    index += sprintf(sgf + index, "HA[%d]\n", last->handicap);
    if (last->result[0] != '\0')
    {
        index += sprintf(sgf + index, "RE[");
        index += sprintf(sgf + index, "%s", last->result);
        index += sprintf(sgf + index, "]\n");
    }
    index += sprintf(sgf + index, "\n");

    int i;
    for (i = 1; i < HistorySize(); ++i)
    {
        current = GetHistory(i);
        Move lm = current->lastmove;
        if (lm.p.col != -1)
        {
            index += sprintf(sgf + index, ";%c[%c%c]",
                    lm.color - 32, 'a' + lm.p.col, 'a' + lm.p.row);
        }
        if (i % 10 == 0)
            index += sprintf(sgf + index, "\n");
    }
    index += sprintf(sgf + index, ")");
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
    {
        if (c != '\n')
            sgf[i++] = c;
    }

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
    *c = '\0';
    strncpy(dst, src, n - 1);
    *c = ']';
    return c + 1;
}

// Return a pointer to the beginning of a label following a tag
char* FindNextLabel(char* src)
{
    char* c = FindTagEnd(src);
    return c + 1;
}

// Return a pointer to the opening '[' of a tag
char* FindNextTag(char* src)
{
    char* c = src;
    while ((*c != '[') && (*c != '\0'))
        c++;
    return c;
}

void LoadSGF(Goban* goban, char* sgf)
{
    // Start from clean state
    ResetGoban(goban);
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
            bzero(goban->whitename, 100);
            CopyTagContents(goban->whitename, label_end + 1, 100);
        }
        else if (!strncmp(label, "PB", label_end - label))
        {
            bzero(goban->blackname, 100);
            CopyTagContents(goban->blackname, label_end + 1, 100);
        }
        else if (!strncmp(label, "KM", label_end - label))
        {
            char* tag_end = FindTagEnd(label_end);
            *tag_end = '\0';
            goban->komi = strtof(label_end + 1, NULL);
            *tag_end = ']';
        }
        else if (!strncmp(label, "SZ", label_end - label))
        {
            char* tag_end = FindTagEnd(label_end);
            *tag_end = '\0';
            goban->size = strtod(label_end + 1, NULL);
            *tag_end = ']';
        }
        else if (!strncmp(label, "HA", label_end - label))
        {
            char* tag_end = FindTagEnd(label_end);
            *tag_end = '\0';
            SetHandicap(goban, strtod(label_end + 1, NULL));
            *tag_end = ']';
        }
        else if (!strncmp(label, "RE", label_end - label))
        {
            bzero(goban->result, 10);
            CopyTagContents(goban->result, label_end + 1, 10);
        }
        label = FindNextLabel(label_end);
        label_end = FindNextTag(label);
    } 

    // Read Moves
    while ((token = strtok_r(NULL, ";", &save_ptr)) != NULL)
    {
        label = token;
        label_end = FindNextTag(label);
        while (label[0] != '\0')
        {
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
          label = FindNextLabel(label);
          label_end = FindNextTag(label);
        }
    }
    sprintf(goban->notes, "Loaded game from sgf...\n");
}
